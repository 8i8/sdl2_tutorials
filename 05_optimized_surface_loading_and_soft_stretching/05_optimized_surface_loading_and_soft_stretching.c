/*
 * This program demonstrates the rescaling of an image by way of the
 * SDL_BlitScaled() function.
 *
 * https://wiki.libsdl.org/SDL_BlitScaled
 *
 * Note: SDL_SWSURFACE was added to SDL_ConvertSurface in place of the NULL
 * value that was present on downloading ...
 */
#include <SDL2/SDL.h>
#include <stdio.h>

const int SCREEN_WIDTH = 640;
const int SCREEN_HEIGHT = 480;

SDL_Surface* loadSurface(char *str);
SDL_Window* gWindow = NULL;
SDL_Surface* gScreenSurface = NULL;
SDL_Surface* gStretchedSurface = NULL;

int init()
{
	if(SDL_Init(SDL_INIT_VIDEO) < 0) {
		SDL_Log("%s(), SDL_Init failed. %s", __func__, SDL_GetError());
		return -1;
	}

	gWindow = SDL_CreateWindow(
					"SDL Tutorial +",
					SDL_WINDOWPOS_UNDEFINED,
					SDL_WINDOWPOS_UNDEFINED,
					SCREEN_WIDTH, SCREEN_HEIGHT,
					SDL_WINDOW_SHOWN);
	if(gWindow == NULL) {
		SDL_Log("%s(), SDL_CreateWindow failed.", __func__);
		return -1;
	}

	gScreenSurface = SDL_GetWindowSurface(gWindow);

	return 0;
}

/*
 * Load the bitmap onto an SDL surface.
 */
int loadMedia()
{
	if((gStretchedSurface = loadSurface("stretch.bmp")) == NULL)
		return -1;

	return 0;
}

/*
 * Close all opened SDL objects and free any memory allocated.
 */
void close_all()
{
	SDL_FreeSurface(gStretchedSurface);
	gStretchedSurface = NULL;
	SDL_DestroyWindow(gWindow);
	gWindow = NULL;
	SDL_Quit();
}

/*
 * The bitmap image on the surface is loaded into the SDL_ConvertSurface
 * function.
 *
 * https://wiki.libsdl.org/SDL_ConvertSurface
 */
SDL_Surface* loadSurface(char *path)
{
	SDL_Surface* optimizedSurface = NULL;

	SDL_Surface* loadedSurface = SDL_LoadBMP((char*)path);
	if(loadedSurface == NULL) {
		SDL_Log("%s(), SDL_LoadBMP failed.", __func__);
		return NULL;
	}

	optimizedSurface = SDL_ConvertSurface(
						loadedSurface,
						gScreenSurface->format,
						SDL_SWSURFACE);
	if(optimizedSurface == NULL) {
		SDL_Log("%s(), SDL_ConvertSurface failed.", __func__);
		return NULL;
	}

	SDL_FreeSurface(loadedSurface);

	return optimizedSurface;
}

/*
 * The scaled image is blitted to the screen by the SDL_BlitScaled() function.
 *
 * https://wiki.libsdl.org/SDL_BlitScaled
 */
int main(int argc, char* args[])
{
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

		SDL_Rect stretchRect;
		stretchRect.x = 10;
		stretchRect.y = 10;
		stretchRect.w = SCREEN_WIDTH-20;
		stretchRect.h = SCREEN_HEIGHT-20;
		SDL_BlitScaled(
				gStretchedSurface,
				NULL,
				gScreenSurface,
				&stretchRect);
	
		SDL_UpdateWindowSurface(gWindow);
	}
equit:
	close_all();

	return 0;
}

