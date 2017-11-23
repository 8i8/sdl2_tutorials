/*
 * This program shows thow to use SDL's accelaerated 2d texture rendering
 * capability, a new fast tecnique for rendering to screen.
 * This hint is checked when a texture is created and it affects scaling when
 * copying that texture. 
 *
 * https://wiki.libsdl.org/SDL_HINT_RENDER_SCALE_QUALITY
 *
 * 	0 or nearest 	→	nearest pixel sampling
 * 	1 or linear	→	linear filtering (supported by OpenGL and Direct3D)
 *	2 or best	→	anisotropic filtering (supported by Direct3D)
 */
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <stdio.h>

const int SCREEN_WIDTH = 640;
const int SCREEN_HEIGHT = 480;

SDL_Texture* loadTexture(char *path);
SDL_Window* gWindow = NULL;
SDL_Renderer* gRenderer = NULL;
SDL_Texture* gTexture = NULL;

short init()
{
	if(SDL_Init(SDL_INIT_VIDEO) < 0) {
		SDL_Log("%s(), SDL_Init failed.", __func__);
		return -1;
	}

	if(SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "1") == 0)
		SDL_Log("%s() Linear texture filtering not enabled.",
				__func__);

	gWindow = SDL_CreateWindow(
					"SDL Tutorial",
					SDL_WINDOWPOS_UNDEFINED,
					SDL_WINDOWPOS_UNDEFINED,
					SCREEN_WIDTH,
					SCREEN_HEIGHT,
					SDL_WINDOW_SHOWN);
	if(gWindow == NULL) {
		SDL_Log("%s(): SDL_CreateWindow failed.", __func__);
		return -1;
	}

	gRenderer = SDL_CreateRenderer(gWindow, -1, SDL_RENDERER_ACCELERATED);

	if(gRenderer == NULL) {
		SDL_Log("%s(): SDL_CreateRenderer failed.", __func__);
		return -1;
	}

	SDL_SetRenderDrawColor(gRenderer, 0xFF, 0xFF, 0xFF, 0xFF);

	if((IMG_Init(IMG_INIT_PNG) & IMG_INIT_PNG) == 0) {
		SDL_Log("%s(): IMG_Init failed.", __func__);
		return -1;
	}

	return 0;
}

short loadMedia()
{
	if((gTexture = loadTexture((char*)"texture.png")) == NULL)
		return -1;

	return 0;
}

SDL_Texture* loadTexture(char *path)
{
	
	SDL_Texture* newTexture = NULL;
	SDL_Surface* loadedSurface;
	
	if((loadedSurface = IMG_Load((char*)path)) == NULL) {
		SDL_Log("%s(): IMG_Load failed.", __func__);
		return NULL;
	}

	if((newTexture = SDL_CreateTextureFromSurface(
					gRenderer, loadedSurface)) == NULL) {
		SDL_Log("%s(): SDL_CreateTextureFromSurface failed.",
				__func__);
		return NULL;
	}
	

	SDL_FreeSurface(loadedSurface);

	return newTexture;
}

void close_all()
{
	SDL_DestroyTexture(gTexture);
	gTexture = NULL;
	SDL_DestroyRenderer(gRenderer);
	gRenderer = NULL;
	SDL_DestroyWindow(gWindow);
	gWindow = NULL;

	IMG_Quit();
	SDL_Quit();
}

int main(int argc, char* argv[])
{
	SDL_Event e;

	if(init()) {
		SDL_Log("error: %s(), %s\n", __func__, SDL_GetError());
		goto equit;
	}

	if(loadMedia()) {
		SDL_Log("error: %s(), %s\n", __func__, SDL_GetError());
		goto equit;
	}

	while(1) {
		while(SDL_PollEvent(&e) != 0)
			if(e.type == SDL_QUIT)
				goto equit;
		
		SDL_RenderClear(gRenderer);
		SDL_RenderCopy(gRenderer, gTexture, NULL, NULL);
		SDL_RenderPresent(gRenderer);
	}
equit:
	close_all();

	return 0;
}
