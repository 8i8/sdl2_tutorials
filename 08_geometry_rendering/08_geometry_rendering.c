/*
 * This program renders some basic geometrical shapes to the screen.
 */
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <stdio.h>
#include <math.h>

const int SCREEN_WIDTH = 640;
const int SCREEN_HEIGHT = 480;

SDL_Window* gWindow = NULL;
SDL_Renderer* gRenderer = NULL;

short init()
{
	if(SDL_Init(SDL_INIT_VIDEO) < 0) {
		SDL_Log("%s(), SDL_Init failed. SDL_Error: \"%s\"",
				__func__, SDL_GetError());
		return -1;
	}

	if(SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "1") == 0)
		SDL_Log("Warning: Linear texture filtering not enabled!");

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

			

	gRenderer = SDL_CreateRenderer(gWindow, -1, SDL_RENDERER_ACCELERATED);
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

void close_all()
{
	SDL_DestroyRenderer(gRenderer);
	SDL_DestroyWindow(gWindow);
	gWindow = NULL;
	gRenderer = NULL;
	IMG_Quit();
	SDL_Quit();
}

/*
 * Render a solid rectangle.
 */
void red_rectangle()
{
	/* 
	 * x, y (upperleft most point)
	 * width, height
	 */
	SDL_Rect fillRect = {
				SCREEN_WIDTH	/ 4,
				SCREEN_HEIGHT	/ 4,
				SCREEN_WIDTH	/ 2,
				SCREEN_HEIGHT	/ 2
	};
	/* colour, red green blue and alpha */
	SDL_SetRenderDrawColor(gRenderer, 0xFF, 0x00, 0x00, 0xFF);
	/* shape */
	SDL_RenderFillRect(gRenderer, &fillRect);
}

/*
 * Render a rectangle frame.
 */
void outline_rectangle()
{
	SDL_Rect outlineRect = {
				SCREEN_WIDTH	/ 6,
				SCREEN_HEIGHT	/ 6,
				SCREEN_WIDTH	* 2 / 3,
				SCREEN_HEIGHT	* 2 / 3
	};
	SDL_SetRenderDrawColor(gRenderer, 0x00, 0xFF, 0x00, 0xFF);		
	SDL_RenderDrawRect(gRenderer, &outlineRect);
}

void horizontal_blue_line()
{
	SDL_SetRenderDrawColor(gRenderer, 0x00, 0x00, 0xFF, 0xFF);		
	SDL_RenderDrawLine(
				gRenderer,
				0,
				SCREEN_HEIGHT / 2,
				SCREEN_WIDTH,
				SCREEN_HEIGHT / 2);
}

void yellow_dotted_line()
{
	int i;
	SDL_SetRenderDrawColor(gRenderer, 0xFF, 0xFF, 0x00, 0xFF);
	for(i = 0; i < SCREEN_HEIGHT; i += 4) {
		SDL_RenderDrawPoint(gRenderer, SCREEN_WIDTH / 2,	i);
		SDL_RenderDrawPoint(gRenderer, SCREEN_WIDTH / 2+1, 	i);
		SDL_RenderDrawPoint(gRenderer, SCREEN_WIDTH / 2, 	i+1);
		SDL_RenderDrawPoint(gRenderer, SCREEN_WIDTH / 2+1,	i+1);
	}
}

void my_points(int X, int Y, int Z)
{
	int i, j;
	SDL_SetRenderDrawColor(gRenderer, 0x00, 0x00, 0x00, 0xFF);

	for (i = 0 ; i < Z; i++)
		for (j = 0 ; j < Z; j++)
			SDL_RenderDrawPoint(gRenderer, X+i, Y+j);
}

int main(int argc, char* args[])
{
	SDL_Event e;

	if(init())
		goto equit;
	
	int x, y;
	x = 10, y = 10;
	while(1)
	{
		while(SDL_PollEvent(&e) != 0)
			if(e.type == SDL_QUIT)
				goto equit;

		SDL_SetRenderDrawColor(gRenderer, 0xFF, 0xFF, 0xFF, 0xFF);
		SDL_RenderClear(gRenderer);

		red_rectangle();
		outline_rectangle();
		horizontal_blue_line();
		yellow_dotted_line();

		my_points(x, y, 6);
		if((x += 4)  >= SCREEN_WIDTH)
			x = 0;
		if(++y == SCREEN_HEIGHT)
			y = 0;

		SDL_RenderPresent(gRenderer);

		SDL_Delay(4);
	}
equit:
	close_all();

	return 0;
}

