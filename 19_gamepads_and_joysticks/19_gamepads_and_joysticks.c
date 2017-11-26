/*
 * This programme demonstrates the use of a joystick, using SDL_INIT_JOYSTICK
 * flag on SDL_Init and the SDL_Event SDL_JOYAXISMOTION flag.
 *
 * https://wiki.libsdl.org/SDL_SetHint
 */
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <stdio.h>
#include <math.h>

const int SCREEN_WIDTH = 640;
const int SCREEN_HEIGHT = 480;
const int JOYSTICK_DEAD_ZONE = 8000;

typedef struct {
	SDL_Texture *mTexture;
	int mWidth;
	int mHeight;
} LTexture;

SDL_Window* gWindow = NULL;
SDL_Renderer* gRenderer = NULL;
LTexture gArrowTexture;
SDL_Joystick* gGameController = NULL;

short init()
{
	if(SDL_Init(SDL_INIT_VIDEO) < 0) {
		SDL_Log("%s(), SDL_Init failed. %s", __func__, SDL_GetError());
		return -1;
	}

	if(SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "2") == 0)
		SDL_Log("%s(), Warning: Linear texture filtering disabled",
				__func__);

	if(SDL_NumJoysticks() < 1) {
		SDL_Log("%s(), Warning: No input device connected.",
				__func__);
		return -1;
	}

	gGameController = SDL_JoystickOpen(0);
	if(gGameController == NULL) {
		SDL_Log("%s(), Warning: Unable to open game controller.",
				__func__);
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
	if(LTexture_loadFromFile(&gArrowTexture, "arrow.png"))
		return -1;

	return 0;
}

void close_all()
{
	free_texture(&gArrowTexture);

	SDL_JoystickClose(gGameController);
	gGameController = NULL;

	SDL_DestroyRenderer(gRenderer);
	SDL_DestroyWindow(gWindow);
	gWindow = NULL;
	gRenderer = NULL;

	IMG_Quit();
	SDL_Quit();
}

void joystick_input(SDL_Event *e, int *xDir, int *yDir)
{
	if(e->jaxis.which == 0) {
		if(e->jaxis.axis == 0)
		{
			if(e->jaxis.value < -JOYSTICK_DEAD_ZONE)
				*xDir = -1;
			else if(e->jaxis.value > JOYSTICK_DEAD_ZONE)
				*xDir =  1;
			else
				*xDir = 0;
		}
		else if(e->jaxis.axis == 1)
		{
			if(e->jaxis.value < -JOYSTICK_DEAD_ZONE)
				*yDir = -1;
			else if(e->jaxis.value > JOYSTICK_DEAD_ZONE)
				*yDir =  1;
			else
				*yDir = 0;
		}
	}
}

int main(int argc, char* argv[])
{
	SDL_Event e;
	int xDir = 0;
	int yDir = 0;
	double joystickAngle;

	if(init())
		goto equit;

	if(loadMedia())
		goto equit;

	while(1)
	{
		while(SDL_PollEvent(&e) != 0)
		{
			if(e.type == SDL_QUIT)
				goto equit;

			if(e.type == SDL_JOYAXISMOTION)
				joystick_input(&e, &xDir, &yDir);
		}

		SDL_SetRenderDrawColor(gRenderer, 0xFF, 0xFF, 0xFF, 0xFF);
		SDL_RenderClear(gRenderer);

		joystickAngle = atan2((float)yDir,(float)xDir) * (180.0 / M_PI);
		
		if(xDir == 0 && yDir == 0)
			joystickAngle = 0;

		LTexture_render(
				&gArrowTexture,
				(SCREEN_WIDTH - LTexture_getWidth(
						&gArrowTexture)) / 2,
				(SCREEN_HEIGHT - LTexture_getHeight(
						&gArrowTexture)) / 2,
				NULL,
				joystickAngle,
				NULL,
				SDL_FLIP_NONE);

		SDL_RenderPresent(gRenderer);

		SDL_Delay(16);
	}
equit:
	close_all();

	return 0;
}

