/*
 * The sdl surface is an image that is used to store and image or a frame for
 * blitting (sending) to the screen. The surface does not use hardware
 * rendering and is the easyest way to put an image to screen with SDL.
 */
#include <SDL2/SDL.h>
#include <stdio.h>

const int SCREEN_WIDTH = 640;
const int SCREEN_HEIGHT = 480;

SDL_Window* wWindow = NULL;
SDL_Surface* sMainSurface = NULL;
SDL_Surface* sHelloWorld = NULL;

/*
 * SDL is initiated and the program window created, the current surface
 * associated with the window is assigned to our surface for writing but the
 * SDL_GetWindowSurface function.
 *
 * https://wiki.libsdl.org/SDL_GetWindowSurface
 */
short init()
{
	if(SDL_Init(SDL_INIT_VIDEO) < 0) {
		SDL_Log("%s(), SDL_Init failed. %s", __func__, SDL_GetError());
		return -1;
	}

	wWindow = SDL_CreateWindow(
					"SDL Tutorial",
					SDL_WINDOWPOS_UNDEFINED,
					SDL_WINDOWPOS_UNDEFINED,
					SCREEN_WIDTH, SCREEN_HEIGHT,
					SDL_WINDOW_SHOWN);
	if(wWindow == NULL) {
		SDL_Log("%s(), SDL_CreateWindow failed.", __func__);
		return -1;
	}
	
	sMainSurface = SDL_GetWindowSurface(wWindow);

	return 0;
}

/*
 * The bitmap that is the file hello_world.bmp is drawn onto the sHelloWorld
 * surface; Our image is now inside of the program scope.
 *
 * https://wiki.libsdl.org/SDL_LoadBMP
 */
short loadMedia()
{
	sHelloWorld = SDL_LoadBMP("hello_world.bmp");

	if(sHelloWorld == NULL) {
		SDL_Log("%s(), SDL_LoadBMP failed.", __func__);
		return -1;
	}

	return 0;
}

void close_all()
{
	SDL_FreeSurface(sHelloWorld);
	sHelloWorld = NULL;

	SDL_DestroyWindow(wWindow);
	wWindow = NULL;

	SDL_Quit();
}

/*
 * Our main surface, that which is connected to the main window is now writen
 * to with the SDL_BlitSurface function.
 *
 * https://wiki.libsdl.org/SDL_BlitSurface
 */
int main(int argc, char* args[])
{
	if(init())
		goto equit;

	if(loadMedia())
		goto equit;

	SDL_BlitSurface(sHelloWorld, NULL, sMainSurface, NULL);
	SDL_UpdateWindowSurface(wWindow);
	SDL_Delay(2000);
equit:
	close_all();

	return 0;
}
