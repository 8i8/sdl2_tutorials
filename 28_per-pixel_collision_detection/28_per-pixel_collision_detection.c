/*
 * This program demonstrates pixel level collision detection.
 *
 * TODO Note that this implimentation will only work when the DOT_VEL is set to
 * one, if it is set any higher ther is a space between the two dots when a
 * collision is detected.
 */
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <stdio.h>

#define ZONES		11
#define SCREEN_WIDTH	640
#define SCREEN_HEIGHT	480
#define DOT_WIDTH	20
#define DOT_HEIGHT	20
#define DOT_VEL		5

typedef struct {
	SDL_Texture *mTexture;
	int mWidth;
	int mHeight;
} LTexture;

typedef struct {
	int mPosX, mPosY;
	int mVelX, mVelY;
	SDL_Rect mColliders[ZONES];
} Dot;

short checkCollision(SDL_Rect *a, SDL_Rect *b);
void Dot_shiftColliders(Dot *d);

SDL_Window* gWindow = NULL;
SDL_Renderer* gRenderer = NULL;
LTexture gDotTexture;

short init()
{
	if(SDL_Init(SDL_INIT_VIDEO) < 0) {
		SDL_Log("%s(), SDL_Init failed.", __func__);
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
					SDL_RENDERER_ACCELERATED);
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

	d->mVelX = 0;
	d->mVelY = 0;

	d->mColliders[0].w = 6;
	d->mColliders[0].h = 1;
	d->mColliders[1].w = 10;
	d->mColliders[1].h = 1;
	d->mColliders[2].w = 14;
	d->mColliders[2].h = 1;
	d->mColliders[3].w = 16;
	d->mColliders[3].h = 2;
	d->mColliders[4].w = 18;
	d->mColliders[4].h = 2;
	d->mColliders[5].w = 20;
	d->mColliders[5].h = 6;
	d->mColliders[6].w = 18;
	d->mColliders[6].h = 2;
	d->mColliders[7].w = 16;
	d->mColliders[7].h = 2;
	d->mColliders[8].w = 14;
	d->mColliders[8].h = 1;
	d->mColliders[9].w = 10;
	d->mColliders[9].h = 1;
	d->mColliders[10].w = 6;
	d->mColliders[10].h = 1;

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
			case SDLK_DOWN: d->mVelY -= DOT_VEL; break;
			case SDLK_LEFT: d->mVelX += DOT_VEL; break;
			case SDLK_RIGHT:d->mVelX -= DOT_VEL; break;
		}
	}
}

void Dot_move(Dot *d, SDL_Rect *otherColliders)
{
	d->mPosX += d->mVelX;
	Dot_shiftColliders(d);

	if(
			(d->mPosX < 0)
			|| (d->mPosX + DOT_WIDTH > SCREEN_WIDTH)
			|| checkCollision(d->mColliders, otherColliders))
	{
		d->mPosX -= d->mVelX;
		Dot_shiftColliders(d);
	}

	d->mPosY += d->mVelY;
	Dot_shiftColliders(d);

	if(
			(d->mPosY < 0)
			|| (d->mPosY + DOT_HEIGHT > SCREEN_HEIGHT)
			|| checkCollision(d->mColliders, otherColliders))
	{
		d->mPosY -= d->mVelY;
		Dot_shiftColliders(d);
	}
}

void Dot_render(Dot *d)
{
	LTexture_render(&gDotTexture, d->mPosX, d->mPosY, NULL);
}

void Dot_shiftColliders(Dot *d)
{
	int zone, r;
	r = 0;

	for(zone = 0; zone < ZONES; ++zone)
	{
		d->mColliders[zone].x =
			d->mPosX + (DOT_WIDTH - d->mColliders[zone].w) / 2;

		d->mColliders[zone].y = d->mPosY + r;

		r += d->mColliders[zone].h;
	}
}

short loadMedia()
{
	if(LTexture_loadFromFile(&gDotTexture, "dot.bmp"))
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

short checkCollision(SDL_Rect *a, SDL_Rect *b)
{
	int leftA, leftB;
	int rightA, rightB;
	int topA, topB;
	int bottomA, bottomB;
	int Abox, Bbox;

	for(Abox = 0; Abox < ZONES; Abox++ )
	{
		leftA = a[Abox].x;
		rightA = a[Abox].x + a[Abox].w;
		topA = a[Abox].y;
		bottomA = a[Abox].y + a[Abox].h;

		for(Bbox = 0; Bbox < ZONES; Bbox++ )
		{
			leftB = b[Bbox].x;
			rightB = b[Bbox].x + b[Bbox].w;
			topB = b[Bbox].y;
			bottomB = b[Bbox].y + b[Bbox].h;

			if(
					((bottomA <= topB)
					|| (topA >= bottomB)
					|| (rightA <= leftB)
					|| (leftA >= rightB))
					== 0)
				return 1;
		}
	}

	return 0;
}

int main(int argc, char* argv[])
{
	if(init())
		goto equit;

	if(loadMedia())
		goto equit;

	SDL_Event e;

	Dot dot;
	Dot_init(&dot, 0, 0);
	
	Dot otherDot;
	Dot_init(&otherDot, SCREEN_WIDTH / 4, SCREEN_HEIGHT / 4);
	
	while(1)
	{
		while(SDL_PollEvent(&e) != 0) {
			if(e.type == SDL_QUIT)
				goto equit;

			Dot_handleEvent(&dot, &e);
		}

		Dot_move(&dot, otherDot.mColliders);

		SDL_SetRenderDrawColor(gRenderer, 0xFF, 0xFF, 0xFF, 0xFF);
		SDL_RenderClear(gRenderer);
		
		Dot_render(&dot);
		Dot_render(&otherDot);

		SDL_RenderPresent(gRenderer);
		SDL_Delay(16);
	}
equit:
	close_all();

	return 0;
}

