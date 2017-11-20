/*
 * This program demonstrates the use of SDL_RenderSetViewport() to render
 * seperate parts of the screen.
 *
 * https://wiki.libsdl.org/SDL_RenderSetViewport
 * https://wiki.libsdl.org/SDL_GetError
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

int init()
{
	if(SDL_Init(SDL_INIT_VIDEO) < 0) {
		SDL_Log("%s(), SDL_Init failed. SDL_Error: \"%s\"",
				__func__, SDL_GetError());
		return 1; 
	}

	if(SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "1") < 0)
		SDL_Log("warning: %s() Linear texture filtering not enabled.",
				__func__);

	gWindow = SDL_CreateWindow(
					"SDL Tutorial",
					SDL_WINDOWPOS_UNDEFINED,
					SDL_WINDOWPOS_UNDEFINED,
					SCREEN_WIDTH,
					SCREEN_HEIGHT,
					SDL_WINDOW_SHOWN);
	if(gWindow == NULL) {
		SDL_Log("%s() SDL_CreateWindow failed.", __func__);
		return -1;
	}
	
	gRenderer = SDL_CreateRenderer(gWindow, -1, SDL_RENDERER_ACCELERATED);
	if(gRenderer == NULL) {
		SDL_Log("%s() SDL_CreateRenderer failed.", __func__);
		return -1;
	}
	
	SDL_SetRenderDrawColor(gRenderer, 0xFF, 0xFF, 0xFF, 0xFF);

	if((IMG_Init(IMG_INIT_PNG)& IMG_INIT_PNG) == 0) {
		SDL_Log("%s() IMG_Init failed.", __func__);
		return -1;
	}

	return 0;
}

int loadMedia()
{
	if((gTexture = loadTexture((char*)"viewport.png")) == NULL)
		return -1;

	return 0;
}

void close_all()
{
	SDL_DestroyTexture(gTexture);
	gTexture = NULL;

	SDL_DestroyRenderer(gRenderer);
	SDL_DestroyWindow(gWindow);
	gWindow = NULL;
	gRenderer = NULL;

	IMG_Quit();
	SDL_Quit();
}

SDL_Texture* loadTexture(char *path)
{
	SDL_Texture* newTexture = NULL;

	SDL_Surface* loadedSurface = IMG_Load((char*)path);
	if(loadedSurface == NULL) {
		SDL_Log("%s() IMG_Load failed.", __func__);
		return NULL;
	}
	
	newTexture = SDL_CreateTextureFromSurface(gRenderer, loadedSurface);
	if(newTexture == NULL){
		SDL_Log("%s() SDL_CreateTextureFromSurface failed.",
				__func__);
		return NULL;
	}

	SDL_FreeSurface(loadedSurface);

	return newTexture;
}

int main(int argc, char* argv[])
{
	SDL_Event e;
	SDL_Rect topLeftViewport;
	SDL_Rect topRightViewport;
	SDL_Rect bottomViewport;

	if(init())
		goto equit;
	
	if(loadMedia())
		goto equit;
	
	while(1)
	{
		while(SDL_PollEvent(&e)!= 0)
			if(e.type == SDL_QUIT)
				goto equit;

		SDL_SetRenderDrawColor(gRenderer, 0xFF, 0xFF, 0xFF, 0xFF);
		SDL_RenderClear(gRenderer);

		topLeftViewport.x = 0;
		topLeftViewport.y = 0;
		topLeftViewport.w = SCREEN_WIDTH / 2;
		topLeftViewport.h = SCREEN_HEIGHT / 2;
		SDL_RenderSetViewport(gRenderer, &topLeftViewport);
		SDL_RenderCopy(gRenderer, gTexture, NULL, NULL);

		topRightViewport.x = SCREEN_WIDTH / 2;
		topRightViewport.y = 0;
		topRightViewport.w = SCREEN_WIDTH / 2;
		topRightViewport.h = SCREEN_HEIGHT / 2;
		SDL_RenderSetViewport(gRenderer, &topRightViewport);
		SDL_RenderCopy(gRenderer, gTexture, NULL, NULL);

		bottomViewport.x = 0;
		bottomViewport.y = SCREEN_HEIGHT / 2;
		bottomViewport.w = SCREEN_WIDTH;
		bottomViewport.h = SCREEN_HEIGHT / 2;
		SDL_RenderSetViewport(gRenderer, &bottomViewport);
		SDL_RenderCopy(gRenderer, gTexture, NULL, NULL);

		SDL_RenderPresent(gRenderer);
	}
equit:
	close_all();

	return 0;
}

