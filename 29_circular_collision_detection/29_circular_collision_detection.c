/*This source code copyrighted by Lazy Foo' Productions (2004-2015)
and may not be redistributed without written permission.*/

//Using SDL, SDL_image, standard IO, vectors, and strings
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <stdio.h>

#define DOT_WIDTH	20
#define DOT_HEIGHT	20
#define DOT_VEL		1

//Screen dimension constants
const int SCREEN_WIDTH = 640;
const int SCREEN_HEIGHT = 480;

typedef struct {
	int x, y;
	int r;
} Circle;

typedef struct {
	SDL_Texture *mTexture;
	int mWidth;
	int mHeight;
} LTexture;

typedef struct {
	int mPosX, mPosY;
	int mVelX, mVelY;
	//Dot's collision circle
	Circle mCollider;
} Dot;

short check_collision_circ(Circle *a, Circle *b);
short check_collision_rect(Circle *a, SDL_Rect *b);
double distanceSquared(int x1, int y1, int x2, int y2);
void Dot_shiftColliders(Dot *d);

SDL_Window* gWindow = NULL;		//The window we'll be rendering to
SDL_Renderer* gRenderer = NULL;		//The window renderer
LTexture gDotTexture;			//Scene textures

LTexture *free_texture(LTexture *lt)
{
	//Free texture if it exists
	if(lt->mTexture != NULL)
	{
		SDL_DestroyTexture(lt->mTexture);
		lt->mTexture = NULL;
		lt->mWidth = 0;
		lt->mHeight = 0;
	}
	return lt;
}

short LTexture_loadFromFile(LTexture *lt, char *path)
{
	//Get rid of preexisting texture
	free_texture(lt);

	//The final texture
	SDL_Texture* newTexture = NULL;

	//Load image at specified path
	SDL_Surface* loadedSurface = IMG_Load(path);
	if(loadedSurface == NULL) {
		printf("Unable to load image %s! SDL_image Error: %s\n",
				path, IMG_GetError());
		return -1;
	}

	//Color key image
	SDL_SetColorKey(
			loadedSurface,
			SDL_TRUE,
			SDL_MapRGB(loadedSurface->format, 0, 0xFF, 0xFF));

	//Create texture from surface pixels
	newTexture = SDL_CreateTextureFromSurface(gRenderer, loadedSurface);
	if(newTexture == NULL) {
		printf("Unable to create texture from %s! SDL Error: %s\n",
				path, SDL_GetError());
		return 1;
	}

	//Get image dimensions
	lt->mWidth = loadedSurface->w;
	lt->mHeight = loadedSurface->h;

	//Get rid of old loaded surface
	SDL_FreeSurface(loadedSurface);

	//Return success
	lt->mTexture = newTexture;

	return 0;
}

#ifdef _SDL_TTF_H
short LTexture_loadFromRenderedText(
					LTexture *lt,
					char *textureText,
					SDL_Color textColor)
{
	//Get rid of preexisting texture
	free_texture(lt);

	//Render text surface
	SDL_Surface* textSurface = TTF_RenderText_Solid(
						gFont, textureText, textColor);
	if(textSurface == NULL) {
		printf("Unable to render text surface! SDL_ttf Error: %s\n",
				TTF_GetError());
		return -1;
	}

	//Create texture from surface pixels
	lt->mTexture = SDL_CreateTextureFromSurface(gRenderer, textSurface);
	if(lt->mTexture == NULL) {
		printf("Unable to create texture from rendered text! SDL Error: %s\n",
				SDL_GetError() );
		return -1;
	}

	//Get image dimensions
	lt->mWidth = textSurface->w;
	lt->mHeight = textSurface->h;

	//Get rid of old surface
	SDL_FreeSurface(textSurface);
	
	return 0;
}
#endif

void LTexture_setColor(LTexture *lt, Uint8 red, Uint8 green, Uint8 blue)
{
	//Modulate texture rgb
	SDL_SetTextureColorMod(lt->mTexture, red, green, blue);
}

void LTexture_setBlendMode(LTexture *lt, SDL_BlendMode blending)
{
	//Set blending function
	SDL_SetTextureBlendMode(lt->mTexture, blending);
}
		
void LTexture_setAlpha(LTexture *lt, Uint8 alpha)
{
	//Modulate texture alpha
	SDL_SetTextureAlphaMod(lt->mTexture, alpha);
}

short LTexture_render(LTexture *lt, int x, int y, SDL_Rect* clip)
{
	//Set rendering space and render to screen
	SDL_Rect renderQuad = {x, y, lt->mWidth, lt->mHeight};

	//Set clip rendering dimensions
	if(clip != NULL) {
		renderQuad.w = clip->w;
		renderQuad.h = clip->h;
	}

	//Render to screen
	return SDL_RenderCopy(gRenderer, lt->mTexture, clip, &renderQuad);
}

int LTexture_getWidth(LTexture *lt)
{
	return lt->mWidth;
}

int LTexture_getHeight(LTexture *lt)
{
	return lt->mHeight;
}

void Dot_init(Dot *d, int x, int y)
{
	//Initialize the offsets
	d->mPosX = x;
	d->mPosY = y;

	//Set collision circle size
	d->mCollider.r = DOT_WIDTH / 2;

	//Initialize the velocity
	d->mVelX = 0;
	d->mVelY = 0;

	//Move collider relative to the circle
	Dot_shiftColliders(d);
}

void Dot_handleEvent(Dot *d, SDL_Event *e)
{
	if(e->type == SDL_KEYDOWN && e->key.repeat == 0)
	{
		switch(e->key.keysym.sym)
		{
			case SDLK_UP: 	d->mVelY -= DOT_VEL; break;
			case SDLK_DOWN: d->mVelY += DOT_VEL; break;
			case SDLK_LEFT: d->mVelX -= DOT_VEL; break;
			case SDLK_RIGHT:d->mVelX += DOT_VEL; break;
		}
	}
	else if(e->type == SDL_KEYUP && e->key.repeat == 0)
	{
		switch(e->key.keysym.sym)
		{
			case SDLK_UP:	d->mVelY += DOT_VEL; break;
			case SDLK_DOWN:	d->mVelY -= DOT_VEL; break;
			case SDLK_LEFT: d->mVelX += DOT_VEL; break;
			case SDLK_RIGHT:d->mVelX -= DOT_VEL; break;
		}
	}
}

void Dot_move(Dot *d, SDL_Rect *square, Circle *circle)
{
	//Move the dot left or right
	d->mPosX += d->mVelX;
	Dot_shiftColliders(d);

	//If the dot collided or went too far to the left or right
	if(
			(d->mPosX - d->mCollider.r < 0)
			|| (d->mPosX + d->mCollider.r > SCREEN_WIDTH)
			|| check_collision_rect(&d->mCollider, square)
			|| check_collision_circ(&d->mCollider, circle))
	{
		d->mPosX -= d->mVelX;
		Dot_shiftColliders(d);
	}

	//Move the dot up or down
	d->mPosY += d->mVelY;
	Dot_shiftColliders(d);

	//If the dot collided or went too far up or down
	if(
			(d->mPosY - d->mCollider.r < 0)
			|| (d->mPosY + d->mCollider.r > SCREEN_HEIGHT)
			|| check_collision_rect(&d->mCollider, square)
			|| check_collision_circ(&d->mCollider, circle))
	{
		d->mPosY -= d->mVelY;
		Dot_shiftColliders(d);
	}
}

void Dot_render(Dot *d)
{
	LTexture_render(
			&gDotTexture,
			d->mPosX - d->mCollider.r,
			d->mPosY - d->mCollider.r,
			NULL);
}

void Dot_shiftColliders(Dot *d)
{
	//Align collider to center of dot
	d->mCollider.x = d->mPosX;
	d->mCollider.y = d->mPosY;
}

short init()
{
	//Initialize SDL
	if(SDL_Init(SDL_INIT_VIDEO) < 0) {
		printf("SDL could not initialize! SDL Error: %s\n",
				SDL_GetError());
		return -1;
	}

	//Set texture filtering to linear
	if(SDL_SetHint( SDL_HINT_RENDER_SCALE_QUALITY, "1" ) == 0)
		printf("Warning: Linear texture filtering not enabled!");

	//Create window
	gWindow = SDL_CreateWindow(
					"SDL Tutorial",
					SDL_WINDOWPOS_UNDEFINED,
					SDL_WINDOWPOS_UNDEFINED,
					SCREEN_WIDTH,
					SCREEN_HEIGHT,
					SDL_WINDOW_SHOWN);
	if(gWindow == NULL) {
		printf("Window could not be created! SDL Error: %s\n",
				SDL_GetError());
		return -1;
	}

			//Create vsynced renderer for window
	gRenderer = SDL_CreateRenderer(
					gWindow,
					-1,
					SDL_RENDERER_ACCELERATED
					| SDL_RENDERER_PRESENTVSYNC);
	if(gRenderer == NULL) {
		printf("Renderer could not be created! SDL Error: %s\n",
				SDL_GetError());
		return -1;
	}

	//Initialize renderer color
	SDL_SetRenderDrawColor(gRenderer, 0xFF, 0xFF, 0xFF, 0xFF);

	//Initialize PNG loading
	if((IMG_Init(IMG_INIT_PNG) & IMG_INIT_PNG) == 0) {
		printf("SDL_image could not initialize! SDL_image Error: %s\n",
				IMG_GetError());
		return -1;
	}

	return 0;
}

short loadMedia()
{
	if(LTexture_loadFromFile(&gDotTexture, "dot.bmp") < 0) {
		printf("Failed to load dot texture!\n");
		return -1;
	}
	return 0;
}

void close_all()
{
	//Free loaded images
	free_texture(&gDotTexture);

	//Destroy window	
	SDL_DestroyRenderer(gRenderer);
	SDL_DestroyWindow(gWindow);
	gWindow = NULL;
	gRenderer = NULL;

	//Quit SDL subsystems
	IMG_Quit();
	SDL_Quit();
}

short check_collision_circ(Circle *a, Circle *b)
{
	//Calculate total radius squared
	int totalRadiusSquared = a->r + b->r;
	totalRadiusSquared = totalRadiusSquared * totalRadiusSquared;

	//If the distance between the centers of the circles is less than the
	//sum of their radi
	if(distanceSquared( a->x, a->y, b->x, b->y) < (totalRadiusSquared))
		return -1;

	return 0;
}

short check_collision_rect(Circle *a, SDL_Rect *b)
{
	//Closest point on collision box
	int cX, cY;

	//Where is the lateral contact relative to the box, the final condition
	//is inside of it.
	if(a->x < b->x)
		cX = b->x;
	else if(a->x > b->x + b->w)
		cX = b->x + b->w;
	else
		cX = a->x;

	//Where is the vertical contact relative to the box, the final condition
	//is inside of it.
	if(a->y < b->y)
		cY = b->y;
	else if(a->y > b->y + b->h)
		cY = b->y + b->h;
	else
		cY = a->y;

	//If the closest point is inside the circle
	if(distanceSquared(a->x, a->y, cX, cY) < a->r * a->r)
		return -1;

	//If the shapes have not collided
	return 0;
}

double distanceSquared(int x1, int y1, int x2, int y2)
{
	int deltaX = x2 - x1;
	int deltaY = y2 - y1;
	return deltaX*deltaX + deltaY*deltaY;
}

int main(int argc, char* argv[])
{
	//Start up SDL and create window
	if(init()) {
		printf("Failed to initialize!\n");
		goto equit;
	}

	//Load media
	if(loadMedia()) {
		printf("Failed to load media!\n");
		goto equit;
	}

	SDL_Event e;

	//The dot that will be moving around on the screen
	Dot dot;
	Dot otherDot;
	Dot_init(&dot, DOT_WIDTH / 2, DOT_HEIGHT / 2);
	Dot_init(&otherDot, SCREEN_WIDTH / 4, SCREEN_HEIGHT / 4);

	//Set the wall
	SDL_Rect wall;
	wall.x = 300;
	wall.y = 40;
	wall.w = 40;
	wall.h = 400;
	
	//While application is running
	while(1)
	{
		while(SDL_PollEvent(&e) != 0) {
			if(e.type == SDL_QUIT)
				goto equit;

			//Handle input for the dot
			Dot_handleEvent(&dot, &e);
		}

		//Move the dot and check collision
		Dot_move(&dot, &wall, &otherDot.mCollider);

		//Clear screen
		SDL_SetRenderDrawColor(gRenderer, 0xFF, 0xFF, 0xFF, 0xFF);
		SDL_RenderClear(gRenderer);

		//Render wall
		SDL_SetRenderDrawColor(gRenderer, 0x00, 0x00, 0x00, 0xFF);
		SDL_RenderDrawRect(gRenderer, &wall);
		
		//Render dots
		Dot_render(&dot);
		Dot_render(&otherDot);

		//Update screen
		SDL_RenderPresent(gRenderer);
	}
equit:
	//Free resources and close SDL
	close_all();

	return 0;
}
