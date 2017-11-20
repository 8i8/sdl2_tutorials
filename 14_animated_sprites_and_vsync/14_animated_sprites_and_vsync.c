/*
 * This program loads a basic animation using the renderquad and clip input
 * from the SDL_RenderCopy function to render frames from an imported PNG
 * stored in one texture, scrolling through an array of SDL_Rect in order to
 * achieve this.
 */
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <stdio.h>

const int SCREEN_WIDTH = 640;
const int SCREEN_HEIGHT = 480;

typedef struct {
	SDL_Texture *mTexture;
	int mWidth;
	int mHeight;
} LTexture;

SDL_Window* gWindow = NULL;
SDL_Renderer* gRenderer = NULL;

#define WALKING_ANIMATION_FRAMES	4
SDL_Rect gSpriteClips[WALKING_ANIMATION_FRAMES];
LTexture gSpriteSheetTexture;

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
					SDL_RENDERER_ACCELERATED
					| SDL_RENDERER_PRESENTVSYNC);
	if(gWindow == NULL) {
		SDL_Log("%s(), SDL_CreateWindow failed.", __func__);
		return -1;
	}

	if ((IMG_Init(IMG_INIT_PNG) & IMG_INIT_PNG) == 0) {
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

short loadFromFile(LTexture *lt, char *path)
{
	free_texture(lt);

	SDL_Texture* newTexture = NULL;

	SDL_Surface* loadedSurface = IMG_Load(path);
	if(loadedSurface == NULL) {
		SDL_SetError("%s(), IMG_Load failed to load \"%s\".",
				__func__, path);
		return -1;
	}

	SDL_SetColorKey(loadedSurface, SDL_TRUE, SDL_MapRGB(
				loadedSurface->format, 0, 0xFF, 0xFF));

	newTexture = SDL_CreateTextureFromSurface(gRenderer, loadedSurface);
	if(newTexture == NULL) {
		SDL_SetError("%s(), SDL_CreateTextureFromSurface failed.",
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

int LTexture_getWidth(LTexture *lt)
{
	return lt->mWidth;
}

int LTexture_getHeight(LTexture *lt)
{
	return lt->mHeight;
}

short loadMedia()
{
	if(loadFromFile(&gSpriteSheetTexture, "foo.png")) {
		SDL_Log("%s(), %s", __func__, SDL_GetError());
		return -1;
	}

	gSpriteClips[0].x =   0;
	gSpriteClips[0].y =   0;
	gSpriteClips[0].w =  64;
	gSpriteClips[0].h = 205;

	gSpriteClips[1].x =  64;
	gSpriteClips[1].y =   0;
	gSpriteClips[1].w =  64;
	gSpriteClips[1].h = 205;
	
	gSpriteClips[2].x = 128;
	gSpriteClips[2].y =   0;
	gSpriteClips[2].w =  64;
	gSpriteClips[2].h = 205;

	gSpriteClips[3].x = 196;
	gSpriteClips[3].y =   0;
	gSpriteClips[3].w =  64;
	gSpriteClips[3].h = 205;
	
	return 0;
}

void close_all()
{
	free_texture(&gSpriteSheetTexture);

	SDL_DestroyRenderer(gRenderer);
	SDL_DestroyWindow(gWindow);
	gWindow = NULL;
	gRenderer = NULL;

	IMG_Quit();
	SDL_Quit();
}

int main(int argc, char* argv[])
{
	int frame = 0;
	SDL_Event e;

	if(init())
		goto equit;

	if(loadMedia())
		goto equit;

	while(1)
	{
		while(SDL_PollEvent(&e) != 0)
			if(e.type == SDL_QUIT)
				goto equit;
		SDL_SetRenderDrawColor(gRenderer, 0xFF, 0xFF, 0xFF, 0xFF);
		SDL_RenderClear(gRenderer);

		SDL_Rect* currentClip = &gSpriteClips[frame / 4];
		LTexture_render(
					&gSpriteSheetTexture,
					(SCREEN_WIDTH - currentClip->w) / 2,
					(SCREEN_HEIGHT - currentClip->h) / 2,
					currentClip);
		SDL_RenderPresent(gRenderer);

		++frame;

		if(frame / 4 >= WALKING_ANIMATION_FRAMES)
			frame = 0;

		SDL_Delay(30);
	}
equit:
	close_all();

	return 0;
}
