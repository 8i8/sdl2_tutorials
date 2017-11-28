/*
 * This program demonstrates collision detection using a geometrical shape, in
 * this case a circle.
 *
 * TODO work over the math in htis example to understand exactly what is
 * happening and improve the collision detection, as with the pervious example
 * the detection fails when the velocity is greater than one.
 */
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <stdio.h>

#define DOT_WIDTH	20
#define DOT_HEIGHT	20
#define DOT_VEL		5
#define SCREEN_WIDTH	640
#define SCREEN_HEIGHT	480

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
	Circle mCollider;
} Dot;

short check_collision_circ(Circle *a, Circle *b);
short check_collision_rect(Circle *a, SDL_Rect *b);
double distanceSquared(int x1, int y1, int x2, int y2);
void Dot_shiftColliders(Dot *d);

SDL_Window* gWindow = NULL;
SDL_Renderer* gRenderer = NULL;
LTexture gDotTexture;

short init()
{
	if(SDL_Init(SDL_INIT_VIDEO) < 0) {
		SDL_Log("%s(), SDL_Init failed. %s", __func__, SDL_GetError());
		return -1;
	}

	gWindow = SDL_CreateWindow(
					"SDL Tutorial",
					SDL_WINDOWPOS_UNDEFINED,
					SDL_WINDOWPOS_UNDEFINED,
					SCREEN_WIDTH,
					SCREEN_HEIGHT,
					SDL_WINDOW_SHOWN);
	if(gWindow == NULL) {
		SDL_Log("%s(), SDL_CreateWindow failed.", __func__);
		return -1;
	}

	gRenderer = SDL_CreateRenderer(
					gWindow,
					-1,
					SDL_RENDERER_ACCELERATED
					| SDL_RENDERER_PRESENTVSYNC);
	if(gRenderer == NULL) {
		SDL_Log("%s(), SDL_CreateRenderer failed.", __func__);
		return -1;
	}

	SDL_SetRenderDrawColor(gRenderer, 0xFF, 0xFF, 0xFF, 0xFF);

	if((IMG_Init(IMG_INIT_PNG) & IMG_INIT_PNG) == 0) {
		SDL_Log("%s(), IMG_Init failed.", __func__);
		return -1;
	}

	return 0;
}

LTexture *free_texture(LTexture *lt)
{
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
	free_texture(lt);

	SDL_Texture* newTexture = NULL;

	SDL_Surface* loadedSurface = IMG_Load(path);
	if(loadedSurface == NULL) {
		SDL_Log("%s(), IMG_Load failed to load \"%s\".",
				__func__, path);
		return -1;
	}

	SDL_SetColorKey(
			loadedSurface,
			SDL_TRUE,
			SDL_MapRGB(loadedSurface->format, 0, 0xFF, 0xFF));

	newTexture = SDL_CreateTextureFromSurface(gRenderer, loadedSurface);
	if(newTexture == NULL) {
		SDL_Log("%s(), SDL_CreateTextureFromSurface failed.",
				__func__);
		return -1;
	}

	lt->mWidth = loadedSurface->w;
	lt->mHeight = loadedSurface->h;

	SDL_FreeSurface(loadedSurface);

	lt->mTexture = newTexture;

	return 0;
}

short LTexture_render(LTexture *lt, int x, int y, SDL_Rect* clip)
{
	SDL_Rect renderQuad = {x, y, lt->mWidth, lt->mHeight};

	if(clip != NULL) {
		renderQuad.w = clip->w;
		renderQuad.h = clip->h;
	}

	return SDL_RenderCopy(gRenderer, lt->mTexture, clip, &renderQuad);
}

void Dot_init(Dot *d, int x, int y)
{
	d->mPosX = x;
	d->mPosY = y;

	d->mCollider.r = DOT_WIDTH / 2;

	d->mVelX = 0;
	d->mVelY = 0;

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
	d->mPosX += d->mVelX;
	Dot_shiftColliders(d);

	if(
			(d->mPosX - d->mCollider.r < 0)
			|| (d->mPosX + d->mCollider.r > SCREEN_WIDTH)
			|| check_collision_rect(&d->mCollider, square)
			|| check_collision_circ(&d->mCollider, circle))
	{
		d->mPosX -= d->mVelX;
		Dot_shiftColliders(d);
	}

	d->mPosY += d->mVelY;
	Dot_shiftColliders(d);

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
	d->mCollider.x = d->mPosX;
	d->mCollider.y = d->mPosY;
}

short loadMedia()
{
	if(LTexture_loadFromFile(&gDotTexture, "dot.bmp") < 0)
		return -1;
	return 0;
}

void close_all()
{
	free_texture(&gDotTexture);

	SDL_DestroyRenderer(gRenderer);
	SDL_DestroyWindow(gWindow);
	gWindow = NULL;
	gRenderer = NULL;

	IMG_Quit();
	SDL_Quit();
}

short check_collision_circ(Circle *a, Circle *b)
{
	int totalRadiusSquared = a->r + b->r;
	totalRadiusSquared = totalRadiusSquared * totalRadiusSquared;

	if(distanceSquared(a->x, a->y, b->x, b->y) < (totalRadiusSquared))
		return -1;

	return 0;
}

short check_collision_rect(Circle *a, SDL_Rect *b)
{
	int cX, cY;

	if(a->x < b->x)
		cX = b->x;
	else if(a->x > b->x + b->w)
		cX = b->x + b->w;
	else
		cX = a->x;

	if(a->y < b->y)
		cY = b->y;
	else if(a->y > b->y + b->h)
		cY = b->y + b->h;
	else
		cY = a->y;

	if(distanceSquared(a->x, a->y, cX, cY) < a->r * a->r)
		return -1;

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
	if(init())
		goto equit;

	if(loadMedia())
		goto equit;

	SDL_Event e;

	Dot dot;
	Dot otherDot;
	Dot_init(&dot, DOT_WIDTH / 2, DOT_HEIGHT / 2);
	Dot_init(&otherDot, SCREEN_WIDTH / 4, SCREEN_HEIGHT / 4);

	SDL_Rect wall;
	wall.x = 300;
	wall.y = 40;
	wall.w = 40;
	wall.h = 400;
	
	while(1)
	{
		while(SDL_PollEvent(&e) != 0) {
			if(e.type == SDL_QUIT)
				goto equit;

			Dot_handleEvent(&dot, &e);
		}

		Dot_move(&dot, &wall, &otherDot.mCollider);

		SDL_SetRenderDrawColor(gRenderer, 0xFF, 0xFF, 0xFF, 0xFF);
		SDL_RenderClear(gRenderer);

		SDL_SetRenderDrawColor(gRenderer, 0x00, 0x00, 0x00, 0xFF);
		SDL_RenderDrawRect(gRenderer, &wall);
		
		Dot_render(&dot);
		Dot_render(&otherDot);

		SDL_RenderPresent(gRenderer);
	}
equit:
	close_all();

	return 0;
}

