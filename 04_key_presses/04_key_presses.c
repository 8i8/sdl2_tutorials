/*
 * In this program system events are used to read user input from the keyboard
 * to change the image that is currently bing displayed. The different surfaces
 * that are used to hold the differing imahes are stored in an array of
 * SDL_surface's. The desired surface is copied over to an intermediate surface
 * the current surface which is itself part of the main program loop;
 */
#include <SDL2/SDL.h>
#include <stdio.h>
#include <string.h>

const int SCREEN_WIDTH = 640;
const int SCREEN_HEIGHT = 480;

enum KeyPressSurfaces
{
	KEY_PRESS_SURFACE_DEFAULT,
	KEY_PRESS_SURFACE_UP,
	KEY_PRESS_SURFACE_DOWN,
	KEY_PRESS_SURFACE_LEFT,
	KEY_PRESS_SURFACE_RIGHT,
	KEY_PRESS_SURFACE_TOTAL
};

SDL_Surface* loadSurface(char *path);
SDL_Window* gWindow = NULL;
SDL_Surface* gScreenSurface = NULL;
SDL_Surface* gCurrentSurface = NULL;
SDL_Surface* gKeyPressSurfaces[KEY_PRESS_SURFACE_TOTAL];

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

	gScreenSurface = SDL_GetWindowSurface(gWindow);

	return 0;
}

short loadMedia()
{
	gKeyPressSurfaces[KEY_PRESS_SURFACE_DEFAULT] = SDL_LoadBMP((char*)"press.bmp");
	if(gKeyPressSurfaces[KEY_PRESS_SURFACE_DEFAULT] == NULL) {
		SDL_Log("%s(): Failed to load default image.", __func__);
		return -1;
	}

	gKeyPressSurfaces[KEY_PRESS_SURFACE_UP] = SDL_LoadBMP((char*)"up.bmp");
	if(gKeyPressSurfaces[KEY_PRESS_SURFACE_UP] == NULL) {
		SDL_Log("%s(): Failed to load up image.", __func__);
		return -1;
	}

	gKeyPressSurfaces[KEY_PRESS_SURFACE_DOWN] = SDL_LoadBMP((char*)"down.bmp");
	if(gKeyPressSurfaces[KEY_PRESS_SURFACE_DOWN] == NULL) {
		SDL_Log("%s(): Failed to load down image.", __func__);
		return -1;
	}

	gKeyPressSurfaces[KEY_PRESS_SURFACE_LEFT] = SDL_LoadBMP((char*)"left.bmp");
	if(gKeyPressSurfaces[KEY_PRESS_SURFACE_LEFT] == NULL) {
		SDL_Log("%s(): Failed to load left image.", __func__);
		return -1;
	}

	gKeyPressSurfaces[KEY_PRESS_SURFACE_RIGHT] = SDL_LoadBMP((char*)"right.bmp");
	if(gKeyPressSurfaces[KEY_PRESS_SURFACE_RIGHT] == NULL) {
		SDL_Log("%s(): Failed to load right image.", __func__);
		return -1;
	}

	return 0;
}

void close_all()
{
	int i;

	for (i = 0; i < KEY_PRESS_SURFACE_TOTAL; ++i) {
		SDL_FreeSurface(gKeyPressSurfaces[i]);
		gKeyPressSurfaces[i] = NULL;
	}

	SDL_DestroyWindow(gWindow);
	gWindow = NULL;

	SDL_Quit();
}

int get_event(SDL_Event e)
{
	if(e.type == SDL_QUIT)
		return -1;
	if(e.type == SDL_KEYDOWN)
	{
		switch(e.key.keysym.sym)
		{
			case SDLK_UP:
			gCurrentSurface = gKeyPressSurfaces[KEY_PRESS_SURFACE_UP];
			break;

			case SDLK_DOWN:
			gCurrentSurface = gKeyPressSurfaces[KEY_PRESS_SURFACE_DOWN];
			break;

			case SDLK_LEFT:
			gCurrentSurface = gKeyPressSurfaces[KEY_PRESS_SURFACE_LEFT];
			break;

			case SDLK_RIGHT:
			gCurrentSurface = gKeyPressSurfaces[KEY_PRESS_SURFACE_RIGHT];
			break;

			default:
			gCurrentSurface = gKeyPressSurfaces[KEY_PRESS_SURFACE_DEFAULT];
			break;
		}
	}
	return 0;
}

int main(int argc, char* args[])
{
	SDL_Event e;

	if(init())
		goto equit;
	if(loadMedia())
		goto equit;

	gCurrentSurface = gKeyPressSurfaces[KEY_PRESS_SURFACE_DEFAULT];

	while(1)
	{
		while(SDL_PollEvent(&e)!= 0)
			if(get_event(e))
				goto equit;

		SDL_BlitSurface(gCurrentSurface, NULL, gScreenSurface, NULL);
		SDL_UpdateWindowSurface(gWindow);
	}
equit:
	close_all();

	return 0;
}

