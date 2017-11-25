/*
 * This program demonstrates the direct accessing and manipulation of pixels.
 *
 */
/* Note: SDL_SWSURFACE added in place of NULL in SDL_Surface* formattedSurface = 
 * LTexture::loadFromFile on line 132 */
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <stdio.h>

#define SCREEN_WIDTH	640
#define SCREEN_HEIGHT	480

typedef struct {
	SDL_Texture* mTexture;
	void* mPixels;
	int mPitch;
	int mWidth;
	int mHeight;
} LTexture;

SDL_Window* gWindow = NULL;
SDL_Renderer* gRenderer = NULL;
LTexture gFooTexture;

short init()
{
	if(SDL_Init(SDL_INIT_VIDEO) < 0) {
		SDL_Log("%s(), SDL_Init failed.", __func__);
		return -1;
	}

	if(SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "1") == 0)
		SDL_Log("Warning: Linear texture filtering not enabled!");

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
						SDL_GetWindowPixelFormat(gWindow),
						SDL_SWSURFACE);
	if(formattedSurface == NULL) {
		SDL_Log("%s(), SDL_ConvertSurfaceFormat failed.", __func__);
		return -1;
	}

	newTexture = SDL_CreateTexture(
						gRenderer,
						SDL_GetWindowPixelFormat(gWindow),
						SDL_TEXTUREACCESS_STREAMING,
						formattedSurface->w,
						formattedSurface->h);
	if(newTexture == NULL) {
		SDL_Log("%s(), SDL_CreateTextureFromSurface failed.", __func__);
		return -1;
	}

	SDL_LockTexture(newTexture, NULL, &lt->mPixels, &lt->mPitch);

	memcpy(
			lt->mPixels,
			formattedSurface->pixels,
			formattedSurface->pitch * formattedSurface->h);

	SDL_UnlockTexture(newTexture);
	lt->mPixels = NULL;

	lt->mWidth = formattedSurface->w;
	lt->mHeight = formattedSurface->h;

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

int LTexture_getWidth(LTexture *lt)
{
	return lt->mWidth;
}

int LTexture_getHeight(LTexture *lt)
{
	return lt->mHeight;
}

short LTexture_lockTexture(LTexture *lt)
{
	if(lt->mPixels != NULL) {
		SDL_Log("Texture already locked.");
		return -1;
	}

	if(SDL_LockTexture(lt->mTexture, NULL, &lt->mPixels, &lt->mPitch) != 0) {
		SDL_Log("%s(), SDL_LockTexture failed. %s",
				__func__, SDL_GetError());
		return -1;
	}
	return 0;
}

short LTexture_unlockTexture(LTexture *lt)
{
	if(lt->mPixels == NULL) {
		SDL_Log("Texture not locked.");
		return -1;
	}

	SDL_UnlockTexture(lt->mTexture);
	lt->mPixels = NULL;
	lt->mPitch = 0;

	return 0;
}

void* LTexture_getPixels(LTexture *lt)
{
	return lt->mPixels;
}

int LTexture_getPitch(LTexture *lt)
{
	return lt->mPitch;
}

short loadMedia()
{

	if(LTexture_loadFromFile(&gFooTexture, "foo.png"))
		return -1;

	if(LTexture_lockTexture(&gFooTexture))
		return -1;

	Uint32 format = SDL_GetWindowPixelFormat(gWindow);
	SDL_PixelFormat* mappingFormat = SDL_AllocFormat(format);

	Uint32* pixels = (Uint32*)LTexture_getPixels(&gFooTexture);
	int pixelCount = (LTexture_getPitch(&gFooTexture) / 4)
					* LTexture_getHeight(&gFooTexture);

	Uint32 colorKey = SDL_MapRGB(mappingFormat, 0, 0xFF, 0xFF);
	Uint32 transparent = SDL_MapRGBA(mappingFormat, 0xFF, 0xFF, 0xFF, 0x00);

	int i;
	for(i = 0; i < pixelCount; ++i)
		if(pixels[ i ] == colorKey)
			pixels[ i ] = transparent;

	LTexture_unlockTexture(&gFooTexture);

	SDL_FreeFormat(mappingFormat);

	return 0;
}

void close_all()
{
	LTexture_free(&gFooTexture);

	SDL_DestroyRenderer(gRenderer);
	SDL_DestroyWindow(gWindow);
	gWindow = NULL;
	gRenderer = NULL;

	IMG_Quit();
	SDL_Quit();
}

int main(int argc, char* args[])
{
	if(init())
		goto equit;

	if(loadMedia())
		goto equit;

	SDL_Event e;

	while(1)
	{
		while(SDL_PollEvent(&e) != 0)
			if(e.type == SDL_QUIT)
				goto equit;

		SDL_SetRenderDrawColor(gRenderer, 0xFF, 0xFF, 0xFF, 0xFF);
		SDL_RenderClear(gRenderer);

		LTexture_render(
				&gFooTexture,
				(SCREEN_WIDTH - LTexture_getWidth(&gFooTexture)) / 2,
				(SCREEN_HEIGHT - LTexture_getHeight(&gFooTexture)) / 2,
				NULL, 0.0, NULL, 0);

		SDL_RenderPresent(gRenderer);
	}
equit:
	close_all();

	return 0;
}
