/*
 * In order to load any other format than the bitmap we must use a specific
 * library, this program demonstrats the loading of a png image file.
 *
 * https://www.libsdl.org/projects/SDL_image/docs/SDL_image_8.html
 */
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <stdio.h>

const int SCREEN_WIDTH = 640;
const int SCREEN_HEIGHT = 480;

short init();
short loadMedia();

SDL_Surface* loadSurface(char *path);
SDL_Window* gWindow = NULL;
SDL_Surface* gScreenSurface = NULL;
SDL_Surface* gPNGSurface = NULL;

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

	/* The PNG functionality is initalized here */
	if((IMG_Init(IMG_INIT_PNG)& IMG_INIT_PNG) == 0) {
		SDL_Log("%s(), IMG_Init failed.", __func__);
		return -1;
	}

	gScreenSurface = SDL_GetWindowSurface(gWindow);

	return 0;
}

short loadMedia()
{
	if((gPNGSurface = loadSurface("loaded.png")) == NULL)
		return -1;

	return 0;
}

void close_all()
{
	SDL_FreeSurface(gPNGSurface);
	gPNGSurface = NULL;
	SDL_DestroyWindow(gWindow);
	gWindow = NULL;
	IMG_Quit();
	SDL_Quit();
}

/*
 * The IMG_Load function is used to load in a PNG image.
 */
SDL_Surface* loadSurface(char *path)
{
	
	SDL_Surface* optimizedSurface = NULL;
	SDL_Surface* loadedSurface = NULL;

	if((loadedSurface = IMG_Load((char*)path)) == NULL) {
		SDL_Log("%s() IMG_Load failed.", __func__);
		return NULL;
	}

	optimizedSurface = SDL_ConvertSurface(
						loadedSurface,
						gScreenSurface->format,
						SDL_SWSURFACE);
	if(optimizedSurface == NULL) {
		SDL_Log("%s() SDL_ConvertSurface failed.", __func__);
		return NULL;
	}

	SDL_FreeSurface(loadedSurface);

	return optimizedSurface;
}

int main(int argc, char* args[])
{
	SDL_Event e;

	if(init())
		goto equit;

	if(loadMedia())
		goto equit;

	while(1)
	{
		while(SDL_PollEvent(&e)!= 0)
			if(e.type == SDL_QUIT)
				goto equit;

		SDL_BlitSurface(gPNGSurface, NULL, gScreenSurface, NULL);
		SDL_UpdateWindowSurface(gWindow);
	}
equit:
	close_all();

	return 0;
}

