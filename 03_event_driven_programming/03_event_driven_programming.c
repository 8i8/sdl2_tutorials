/*
 * This program demonstrates the use of the SDL_PollEvent function.
 */
#include <SDL2/SDL.h>
#include <stdio.h>

const int SCREEN_WIDTH = 640;
const int SCREEN_HEIGHT = 480;

SDL_Window* gWindow = NULL;
SDL_Surface* gScreenSurface = NULL;
SDL_Surface* gXOut = NULL;

int init()
{
	if(SDL_Init(SDL_INIT_VIDEO) < 0) {
		SDL_Log("error: %s(), SDL_Init failed. %s", __func__,
				SDL_GetError());
		return -1;
	}

	if((gWindow = SDL_CreateWindow(
					"SDL Tutorial",
					SDL_WINDOWPOS_UNDEFINED,
					SDL_WINDOWPOS_UNDEFINED,
					SCREEN_WIDTH,
					SCREEN_HEIGHT,
					SDL_WINDOW_SHOWN
					)) == NULL)
	{
		SDL_Log("%s(): SDL_CreateWindow failed.", __func__);
		return -1;
	}

	gScreenSurface = SDL_GetWindowSurface(gWindow);

	return 0;
}

int loadMedia()
{
	if((gXOut = SDL_LoadBMP("x.bmp")) == NULL) {
		SDL_Log("%s(): SDL_LoadBMP failed.", __func__);
		return -1;
	}

	return 0;
}

void close_all()
{
	SDL_FreeSurface(gXOut);
	gXOut = NULL;
	SDL_DestroyWindow(gWindow);
	gWindow = NULL;
	SDL_Quit();
}

/*
 * When SDL_PollEvent recieves an event if that event is recgnised by the
 * following if statment then that operation is performed.
 */
int main(int argc, char* argv[])
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

		SDL_BlitSurface(gXOut, NULL, gScreenSurface, NULL);
		SDL_UpdateWindowSurface(gWindow);
	}
equit:
	close_all();

	return 0;
}
