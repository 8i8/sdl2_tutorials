/*This source code copyrighted by Lazy Foo' Productions (2004-2015)
and may not be redistributed without written permission.*/

//Using SDL, SDL_image, standard IO, and strings
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <stdio.h>

//Screen dimension constants
const int SCREEN_WIDTH = 640;
const int SCREEN_HEIGHT = 480;

//The dimensions of the dot
static const int DOT_WIDTH = 20;
static const int DOT_HEIGHT = 20;

//Maximum axis velocity of the dot
static const int DOT_VEL = 10;

//Particle count
#define TOTAL_PARTICLES	40
#define P_LIFE		10

typedef struct {
	SDL_Texture* mTexture;
	int mWidth;
	int mHeight;
} LTexture;

typedef struct {
	int mPosX, mPosY;
	int mFrame;
	LTexture *mTexture;
} Particle;

typedef struct {
	Particle* particles[TOTAL_PARTICLES];
	int mPosX, mPosY;
	int mVelX, mVelY;
} Dot;

SDL_Window* gWindow = NULL;		//The window we'll be rendering to
SDL_Renderer* gRenderer = NULL;		//The window renderer
LTexture gDotTexture;			//Scene textures
LTexture gRedTexture;
LTexture gGreenTexture;
LTexture gBlueTexture;
LTexture gShimmerTexture;

void LTexture_free(LTexture *lt)
{
	//Free texture if it exists
	if(lt->mTexture != NULL) {
		SDL_DestroyTexture(lt->mTexture);
		lt->mTexture = NULL;
		lt->mWidth = 0;
		lt->mHeight = 0;
	}
}

short LTexture_loadFromFile(LTexture *lt, char *path)
{
	//Get rid of preexisting texture
	LTexture_free(lt);

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
				path, SDL_GetError() );
		return -1;
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
				char* textureText,
				SDL_Color textColor)
{
	//Get rid of preexisting texture
	LTexture_free(lt);

	//Render text surface
	SDL_Surface* textSurface = TTF_RenderText_Solid(
						gFont,
						textureText.c_str(),
						textColor);
	if(textSurface == NULL) {
		printf("Unable to render text surface! SDL_ttf Error: %s\n",
				TTF_GetError());
		return -1;
	}

	//Create texture from surface pixels
	lt->mTexture = SDL_CreateTextureFromSurface(gRenderer, textSurface);
	if(mTexture == NULL) {
		printf("Unable to create texture from rendered text! SDL Error: %s\n",
				SDL_GetError());
		return -1;
	}

	//Get image dimensions
	lt->mWidth = textSurface->w;
	lt->mHeight = textSurface->h;

	//Get rid of old surface
	SDL_FreeSurface(textSurface);
	
	//Return success
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

void LTexture_render(
			LTexture *lt,
			int x,
			int y,
			SDL_Rect* clip)
{
	//Set rendering space and render to screen
	SDL_Rect renderQuad = { x, y, lt->mWidth, lt->mHeight };

	//Set clip rendering dimensions
	if(clip != NULL) {
		renderQuad.w = clip->w;
		renderQuad.h = clip->h;
	}

	//Render to screen
	SDL_RenderCopy(
			gRenderer,
			lt->mTexture,
			clip,
			&renderQuad);
}

int LTexture_getWidth(LTexture *lt)	
{
	return lt->mWidth;
}

int LTexture_getHeight(LTexture *lt)
{
	return lt->mHeight;
}

void Particle_init(Particle *p, int x, int y)
{
	//Seed random
	srand(SDL_GetTicks());

	//Set offsets
	p->mPosX = x - 5 + (rand() % 25);
	p->mPosY = y - 5 + (rand() % 25);

	//Initialize animation
	p->mFrame = rand() % 5;

	//Set type
	switch(rand() % 3)
	{
		case 0: p->mTexture = &gRedTexture; break;
		case 1: p->mTexture = &gGreenTexture; break;
		case 2: p->mTexture = &gBlueTexture; break;
	}
}

void Particle_render(Particle *p)
{
	//Show image
	LTexture_render(p->mTexture, p->mPosX, p->mPosY, NULL);

	//Show shimmer
	if(p->mFrame % 2 == 0)
		LTexture_render(&gShimmerTexture, p->mPosX, p->mPosY, NULL);

	//Animate
	p->mFrame++;
}

short Particle_isDead(Particle *p)
{
	return p->mFrame > P_LIFE;
}

void Dot_init(Dot *d)
{
	int i;
	//Initialize the offsets
	d->mPosX = 0;
	d->mPosY = 0;

	//Initialize the velocity
	d->mVelX = 0;
	d->mVelY = 0;

	//Initialize particles
	for(i = 0; i < TOTAL_PARTICLES; ++i) {
		d->particles[i] = malloc(sizeof(Particle));
		Particle_init(d->particles[i], d->mPosX, d->mPosY);
	}
}

void Dot_free(Dot *d)
{
	int i;
	for(i = 0; i < TOTAL_PARTICLES; ++i)
		free(d->particles[i]);
}

void Dot_handleEvent(Dot *d, SDL_Event *e)
{
	//If a key was pressed
	if(e->type == SDL_KEYDOWN && e->key.repeat == 0)
	{
		//Adjust the velocity
		switch(e->key.keysym.sym) {
			case SDLK_UP: 	d->mVelY -= DOT_VEL; break;
			case SDLK_DOWN: d->mVelY += DOT_VEL; break;
			case SDLK_LEFT: d->mVelX -= DOT_VEL; break;
			case SDLK_RIGHT:d->mVelX += DOT_VEL; break;
		}
	}
	//If a key was released
	else if(e->type == SDL_KEYUP && e->key.repeat == 0)
	{
		//Adjust the velocity
		switch(e->key.keysym.sym) {
			case SDLK_UP: 	d->mVelY += DOT_VEL; break;
			case SDLK_DOWN: d->mVelY -= DOT_VEL; break;
			case SDLK_LEFT: d->mVelX += DOT_VEL; break;
			case SDLK_RIGHT:d->mVelX -= DOT_VEL; break;
		}
	}
}

void Dot_move(Dot *d)
{
	//Move the dot left or right
	d->mPosX += d->mVelX;

	//If the dot went too far to the left or right
	if((d->mPosX < 0) || (d->mPosX + DOT_WIDTH > SCREEN_WIDTH))
		d->mPosX -= d->mVelX;

	//Move the dot up or down
	d->mPosY += d->mVelY;

	//If the dot went too far up or down
	if((d->mPosY < 0) || (d->mPosY + DOT_HEIGHT > SCREEN_HEIGHT))
		d->mPosY -= d->mVelY;
}

void Dot_renderParticles(Dot *d)
{
	int i;
	//Go through particles
	for(i = 0; i < TOTAL_PARTICLES; ++i) {
		if(Particle_isDead(d->particles[i])) {
			//LTexture_free(d->particles[i]->mTexture);
			Particle_init(d->particles[i], d->mPosX, d->mPosY);
		}
		Particle_render(d->particles[i]);
	}
}

void Dot_render(Dot *d)
{
	//Show the dot
	LTexture_render(&gDotTexture, d->mPosX, d->mPosY, NULL);

	//Show particles on top of dot
	Dot_renderParticles(d);
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
	if(SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "1") == 0)
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

	//Create renderer for window
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
	//Load dot texture
	if(LTexture_loadFromFile(&gDotTexture, "dot.bmp") < 0) {
		printf("Failed to load dot texture!\n");
		return -1;
	}

	//Load red texture
	if(LTexture_loadFromFile(&gRedTexture, "red.bmp") < 0) {
		printf("Failed to load red texture!\n");
		return -1;
	}

	//Load green texture
	if(LTexture_loadFromFile(&gGreenTexture, "green.bmp") < 0) {
		printf( "Failed to load green texture!\n" );
		return -1;
	}

	//Load blue texture
	if(LTexture_loadFromFile(&gBlueTexture, "blue.bmp") < 0) {
		printf( "Failed to load blue texture!\n" );
		return -1;
	}

	//Load shimmer texture
	if(LTexture_loadFromFile(&gShimmerTexture, "shimmer.bmp") < 0) {
		printf("Failed to load shimmer texture!\n");
		return -1;
	}
	
	//Set texture transparency
	LTexture_setAlpha(&gRedTexture, 192);
	LTexture_setAlpha(&gGreenTexture, 192);
	LTexture_setAlpha(&gBlueTexture, 192);
	LTexture_setAlpha(&gShimmerTexture, 192);

	return 0;
}

void close_all(Dot *d)
{
	Dot_free(d);

	//Free loaded images
	LTexture_free(&gDotTexture);
	LTexture_free(&gRedTexture);
	LTexture_free(&gGreenTexture);
	LTexture_free(&gBlueTexture);
	LTexture_free(&gShimmerTexture);

	//Destroy window	
	SDL_DestroyRenderer(gRenderer);
	SDL_DestroyWindow(gWindow);
	gWindow = NULL;
	gRenderer = NULL;

	//Quit SDL subsystems
	IMG_Quit();
	SDL_Quit();
}

int main(int argc, char* argv[])
{
	SDL_Event e;
	Dot dot;

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

	Dot_init(&dot);

	//While application is running
	while(1)
	{
		while(SDL_PollEvent(&e) != 0) {
			if(e.type == SDL_QUIT)
				goto equit;

			//Handle input for the dot
			Dot_handleEvent(&dot, &e);
		}

		//Move the dot
		Dot_move(&dot);

		//Clear screen
		SDL_SetRenderDrawColor(gRenderer, 0xFF, 0xFF, 0xFF, 0xFF);
		SDL_RenderClear(gRenderer);

		//Render objects
		Dot_render(&dot);

		//Update screen
		SDL_RenderPresent(gRenderer);
	}
equit:
	//Free resources and close SDL
	close_all(&dot);

	return 0;
}
