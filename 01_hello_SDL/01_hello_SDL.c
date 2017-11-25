/*
 * A simple window is created using SDL_CreateWindow. Numerous possible options
 * can be set using the available flags .
 *
 * https://wiki.libsdl.org/SDL_CreateWindow
 */
#include <SDL2/SDL.h>
#include <stdio.h>

const int SCREEN_WIDTH = 640;
const int SCREEN_HEIGHT = 480;

int main(int argc, char* args[])
{
	SDL_Event e;

	SDL_Window* wWindow = NULL;
	SDL_Surface* sScreen = NULL;

	if(SDL_Init(SDL_INIT_VIDEO) < 0) {
		SDL_Log("error: %s(): %s\n", __func__, SDL_GetError());
		goto equit;
	}
	
	wWindow = SDL_CreateWindow(
					"SDL Tutorial",
					SDL_WINDOWPOS_UNDEFINED,
					SDL_WINDOWPOS_UNDEFINED,
					SCREEN_WIDTH,
					SCREEN_HEIGHT,
					SDL_WINDOW_SHOWN
					| SDL_WINDOW_RESIZABLE);
	if(wWindow == NULL) {
		SDL_Log("error: %s(): gWindow could not be created.", __func__);
		goto equit;
	}

	while(1)
	{
		while(SDL_PollEvent(&e))
			if(e.type == SDL_QUIT)
				goto equit;
		 
		sScreen = SDL_GetWindowSurface(wWindow);
		SDL_FillRect(sScreen, NULL, SDL_MapRGB(
					sScreen->format, 0xFF, 0xFF, 0xFF));
		SDL_UpdateWindowSurface(wWindow);
		SDL_Delay(60);
	}
equit:
	SDL_DestroyWindow(wWindow);
	SDL_Quit();

	return 0;
}
