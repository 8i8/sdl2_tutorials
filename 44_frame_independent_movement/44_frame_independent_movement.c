/*
 * This program demonstrates the movement of the dot indipendant of the frame
 * rate.
 */
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <stdio.h>

enum truth { false, true };

#define SCREEN_WIDTH	640
#define SCREEN_HEIGHT	480
#define DOT_WIDTH	20
#define DOT_HEIGHT	20
#define DOT_VEL		640

typedef struct {
	SDL_Texture* mTexture;
	void* mPixels;
	int mPitch;
	int mWidth;
	int mHeight;
} LTexture;

typedef struct {
	Uint32 mStartTicks;
	Uint32 mPausedTicks;
	short mPaused;
	short mStarted;
} LTimer;

typedef struct {
	float mPosX, mPosY;
	float mVelX, mVelY;
} Dot;

SDL_Window* gWindow = NULL;
SDL_Renderer* gRenderer = NULL;
LTexture gDotTexture;

short init()
{
	if(SDL_Init(SDL_INIT_VIDEO) < 0) {
		SDL_Log("%s(), SDL_Init failed.", __func__);
		return -1;
	}

	if(SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "1") == 0)
		SDL_Log("%s(), Warning: Linear texture filtering not enabled.",
				__func__);

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

	gRenderer = SDL_CreateRenderer(gWindow, -1, SDL_RENDERER_ACCELERATED);
	if(gRenderer == NULL) {
		SDL_Log("%s(), SDL_CreateRenderer failed.", __func__);
		return -1;
	}

	SDL_SetRenderDrawColor(gRenderer, 0xFF, 0xFF, 0xFF, 0xFF);

	if((IMG_Init(IMG_INIT_PNG) & IMG_INIT_PNG) == 0) {
		SDL_Log("%s(), IMG_Init failed. %s", __func__, IMG_GetError());
		return -1;
	}

	return 0;
}

void LTexture_free(LTexture *lt)
{
	if(lt->mTexture != NULL) {
		SDL_DestroyTexture(lt->mTexture);
		lt->mTexture = NULL;
		lt->mWidth = 0;
		lt->mHeight = 0;
		lt->mPixels = NULL;
		lt->mPitch = 0;
	}
}

short LTexture_loadFromFile(LTexture *lt, char *path)
{
	LTexture_free(lt);

	SDL_Texture* newTexture = NULL;

	SDL_Surface* loadedSurface = IMG_Load(path);
	if(loadedSurface == NULL) {
		SDL_Log("%s(), IMG_Load failed to load \"%s\".",
				__func__, path);
		return -1;
	}

	SDL_Surface* formattedSurface = SDL_ConvertSurfaceFormat(
							loadedSurface,
							SDL_PIXELFORMAT_RGBA8888,
							SDL_SWSURFACE);
	if(formattedSurface == NULL) {
		SDL_Log("%s(), SDL_ConvertSurfaceFormat failed.", __func__);
		return -1;
	}

	newTexture = SDL_CreateTexture(
					gRenderer,
					SDL_PIXELFORMAT_RGBA8888,
					SDL_TEXTUREACCESS_STREAMING,
					formattedSurface->w,
					formattedSurface->h);
	if(newTexture == NULL) {
		SDL_Log("%s(), SDL_CreateTextureFromSurface failed.", __func__);
		return -1;
	}

	SDL_SetTextureBlendMode(newTexture, SDL_BLENDMODE_BLEND);

	SDL_LockTexture(
				newTexture,
				&formattedSurface->clip_rect,
				&lt->mPixels,
				&lt->mPitch);

	memcpy(
			lt->mPixels,
			formattedSurface->pixels,
			formattedSurface->pitch * formattedSurface->h);

	lt->mWidth = formattedSurface->w;
	lt->mHeight = formattedSurface->h;

	Uint32* pixels = (Uint32*)lt->mPixels;
	int pixelCount = (lt->mPitch / 4) * lt->mHeight;

	Uint32 colorKey = SDL_MapRGB(formattedSurface->format, 0, 0xFF, 0xFF);
	Uint32 transparent = SDL_MapRGBA(
			formattedSurface->format, 0x00, 0xFF, 0xFF, 0x00);

	int i;
	for(i = 0; i < pixelCount; ++i)
		if(pixels[i] == colorKey)
			pixels[i] = transparent;

	SDL_UnlockTexture(newTexture);
	lt->mPixels = NULL;

	SDL_FreeSurface(formattedSurface);
		
	SDL_FreeSurface(loadedSurface);

	lt->mTexture = newTexture;

	return 0;
}

void LTexture_render(
			LTexture *lt,
			int x, int y,
			SDL_Rect* clip,
			double angle,
			SDL_Point* center,
			SDL_RendererFlip flip)
{
	SDL_Rect renderQuad = { x, y, lt->mWidth, lt->mHeight };

	if(clip != NULL) {
		renderQuad.w = clip->w;
		renderQuad.h = clip->h;
	}

	SDL_RenderCopyEx(gRenderer, lt->mTexture, clip,
				&renderQuad, angle, center, flip);
}

void LTimer_start(LTimer *t)
{
	t->mStarted = true;
	t->mPaused = false;
	t->mStartTicks = SDL_GetTicks();
	t->mPausedTicks = 0;
}

Uint32 LTimer_getTicks(LTimer *t)
{
	Uint32 time = 0;

	if(t->mStarted) {
		if(t->mPaused)
			time = t->mPausedTicks;
		else
			time = SDL_GetTicks() - t->mStartTicks;
	}

	return time;
}

void Dot_handleEvent(Dot *d, SDL_Event *e)
{
	if(e->type == SDL_KEYDOWN && e->key.repeat == 0) {
		switch(e->key.keysym.sym)
		{
			case SDLK_UP:    d->mVelY -= DOT_VEL; break;
			case SDLK_DOWN:  d->mVelY += DOT_VEL; break;
			case SDLK_LEFT:  d->mVelX -= DOT_VEL; break;
			case SDLK_RIGHT: d->mVelX += DOT_VEL; break;
		}
	}
	else if(e->type == SDL_KEYUP && e->key.repeat == 0) {
		switch(e->key.keysym.sym)
		{
			case SDLK_UP:    d->mVelY += DOT_VEL; break;
			case SDLK_DOWN:  d->mVelY -= DOT_VEL; break;
			case SDLK_LEFT:  d->mVelX += DOT_VEL; break;
			case SDLK_RIGHT: d->mVelX -= DOT_VEL; break;
		}
	}
}

void Dot_move(Dot *d,float timeStep)
{
	d->mPosX += d->mVelX * timeStep;

	if(d->mPosX < 0)
		d->mPosX = 0;
	else if(d->mPosX > SCREEN_WIDTH - DOT_WIDTH)
		d->mPosX = SCREEN_WIDTH - DOT_WIDTH;
	
	d->mPosY += d->mVelY * timeStep;

	if(d->mPosY < 0)
		d->mPosY = 0;
	else if(d->mPosY > SCREEN_HEIGHT - DOT_HEIGHT)
		d->mPosY = SCREEN_HEIGHT - DOT_HEIGHT;
}

void Dot_render(Dot *d)
{
	LTexture_render(&gDotTexture, (int)d->mPosX, (int)d->mPosY,
			NULL, 0.0, NULL, 0);
}

short loadMedia()
{
	if(LTexture_loadFromFile(&gDotTexture, "dot.bmp"))
		return -1;
	return 0;
}

void close_all()
{
	LTexture_free(&gDotTexture);

	SDL_DestroyRenderer(gRenderer);
	SDL_DestroyWindow(gWindow);
	gWindow = NULL;
	gRenderer = NULL;

	IMG_Quit();
	SDL_Quit();
}

int main(int argc, char* args[])
{
	Dot dot;
	LTimer stepTimer;

	if(init())
		goto equit;

	if(loadMedia())
		goto equit;

	SDL_Event e;

	while(1)
	{
		while(SDL_PollEvent(&e) != 0)
		{
			if(e.type == SDL_QUIT)
				goto equit;

			Dot_handleEvent(&dot, &e);
		}

		float timeStep = LTimer_getTicks(&stepTimer) / 1000.f;
		Dot_move(&dot, timeStep);

		LTimer_start(&stepTimer);

		SDL_SetRenderDrawColor(gRenderer, 0xFF, 0xFF, 0xFF, 0xFF);
		SDL_RenderClear(gRenderer);

		Dot_render(&dot);

		SDL_RenderPresent(gRenderer);
	}
equit:
	close_all();

	return 0;
}
