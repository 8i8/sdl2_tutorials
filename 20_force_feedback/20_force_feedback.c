/*
 * This program demonstrates the use of force feedback from a joystic.
 */
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <stdio.h>
#include <math.h>

const int SCREEN_WIDTH = 640;
const int SCREEN_HEIGHT = 480;

typedef struct {
	SDL_Texture *mTexture;
	int mWidth;
	int mHeight;
} LTexture;

SDL_Window* gWindow = NULL;
SDL_Renderer* gRenderer = NULL;
LTexture gSplashTexture;

SDL_Joystick* gGameController = NULL;
SDL_Haptic* gControllerHaptic = NULL;

short init()
{
	if(SDL_Init(SDL_INIT_VIDEO | SDL_INIT_JOYSTICK | SDL_INIT_HAPTIC)) {
		SDL_Log("%s(), SDL_Init failed. %s", __func__, SDL_GetError());
		return -1;
	}

	if(SDL_NumJoysticks() < 1) {
		SDL_Log("%s(), Warning: No input device connected.", __func__);
		return -1;
	}

	gGameController = SDL_JoystickOpen(0);
	if(gGameController == NULL) {
		SDL_Log("%s(), SDL_JoystickOpen failed.", __func__);
		return -1;
	}

	gControllerHaptic = SDL_HapticOpenFromJoystick(gGameController);
	if(gControllerHaptic == NULL) {
		SDL_Log("%s(), SDL_HapticOpenFromJoystick failed.", __func__);
		return -1;
	}

	if(SDL_HapticRumbleInit(gControllerHaptic) < 0) {
		SDL_Log("%s(), SDL_HapticRumbleInit failed.", __func__);
		return -1;
	}

	gWindow = SDL_CreateWindow(
					"SDL Tutorial",
					SDL_WINDOWPOS_UNDEFINED,
					SDL_WINDOWPOS_UNDEFINED,
					SCREEN_WIDTH, SCREEN_HEIGHT,
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
	if(lt->mTexture != NULL) {
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
		SDL_Log("%s(), SDL_CreateTextureFromSurface failed.", __func__);
		return -1;
	}

	lt->mWidth = loadedSurface->w;
	lt->mHeight = loadedSurface->h;

	SDL_FreeSurface(loadedSurface);

	lt->mTexture = newTexture;

	return 0;
}

short LTexture_render(
			LTexture *lt,
			int x,
			int y,
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

	return SDL_RenderCopyEx(gRenderer, lt->mTexture, clip,
				&renderQuad, angle, center, flip);
}

short loadMedia()
{
	if(LTexture_loadFromFile(&gSplashTexture, "splash.png"))
		return -1;

	return 0;
}

void close_all()
{
	free_texture(&gSplashTexture);

	SDL_HapticClose(gControllerHaptic);
	SDL_JoystickClose(gGameController);
	gGameController = NULL;
	gControllerHaptic = NULL;

	SDL_DestroyRenderer(gRenderer);
	SDL_DestroyWindow(gWindow);
	gWindow = NULL;
	gRenderer = NULL;

	IMG_Quit();
	SDL_Quit();
}

void joystic_rumble()
{
	if(SDL_HapticRumblePlay(gControllerHaptic, 0.75, 500))
		SDL_Log("%s(), SDL_HapticRumblePlay failed. %s",
				__func__, SDL_GetError());
}

int main(int argc, char* argv[])
{
	SDL_Event e;
			
	if(init())
		goto equit;

	if(loadMedia())
		goto equit;

	while(1)
	{
		while(SDL_PollEvent(&e) != 0)
		{
			if( e.type == SDL_QUIT )
				goto equit;

			if(e.type == SDL_JOYBUTTONDOWN)
				joystic_rumble();
		}

		SDL_SetRenderDrawColor(gRenderer, 0xFF, 0xFF, 0xFF, 0xFF);
		SDL_RenderClear(gRenderer);

		LTexture_render(&gSplashTexture, 0, 0, NULL, 0.0, NULL,
						SDL_FLIP_NONE);

		SDL_RenderPresent(gRenderer);
	}
equit:
	close_all();

	return 0;
}
