/*
 * The Viewport
 *
 * Some times you only want to render part of the screen for things like
 * minimaps. Using the viewport you can control where you render on the screen.
 */
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>

#define SCREEN_WIDTH	640
#define SCREEN_HEIGHT	480

SDL_Texture* loadTexture(char *path);
SDL_Window* gWindow = NULL;
SDL_Renderer* gRenderer = NULL;
SDL_Texture* gTexture = NULL;

int init(void)
{
	if(SDL_Init(SDL_INIT_VIDEO) < 0) {
		SDL_Log("%s(), SDL_Init failed. %s", __func__, SDL_GetError());
		return 1; 
	}

	if(SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "1") == 0)
		SDL_Log("Warning: Linear texture filtering not enabled.");

	gWindow = SDL_CreateWindow(
					"SDL Tutorial",
					SDL_WINDOWPOS_UNDEFINED,
					SDL_WINDOWPOS_UNDEFINED,
					SCREEN_WIDTH,
					SCREEN_HEIGHT,
					SDL_WINDOW_SHOWN);
	if(gWindow == NULL) {
		SDL_Log("%s(), SDL_CreateWindow failed. %s", __func__, SDL_GetError());
		return -1;
	}
	
	gRenderer = SDL_CreateRenderer(gWindow, -1, SDL_RENDERER_ACCELERATED);
	if(gRenderer == NULL) {
		SDL_Log("%s(), SDL_CreateRenderer failed. %s", __func__, SDL_GetError());
		return -1;
	}
	
	SDL_SetRenderDrawColor(gRenderer, 0xFF, 0xFF, 0xFF, 0xFF);

	if((IMG_Init(IMG_INIT_PNG)& IMG_INIT_PNG) == 0) {
		SDL_Log("%s(), IMG_Init failed. %s", __func__, IMG_GetError());
		return -1;
	}

	return 0;
}

int loadMedia(void)
{
	if((gTexture = loadTexture((char*)"viewport.png")) == NULL)
		return -1;

	return 0;
}

void close_all(void)
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

	SDL_Surface* loadedSurface = IMG_Load(path);
	if(loadedSurface == NULL) {
		SDL_Log("%s(), IMG_Load failed. %s", __func__, IMG_GetError());
		return NULL;
	}
	
	newTexture = SDL_CreateTextureFromSurface(gRenderer, loadedSurface);
	if(newTexture == NULL){
		SDL_Log("%s(), SDL_CreateTextureFromSurface failed. %s", __func__, SDL_GetError());
		return NULL;
	}

	SDL_FreeSurface(loadedSurface);

	return newTexture;
}

/*
 * After we cleared the screen, it's time to get drawing. There's 3 regions
 * we're going to draw a full screen image to:
 */
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
/*
 * First we're going to render the top left. This is as easy as creating a
 * rectangle with half the width/height as the screen, and passing this region
 * to SDL_RenderSetViewport. Any rendering done after that call will render
 * inside the region defined by the given viewport. It will also use the
 * coordinate system of the window it was created in, so the bottom of the view
 * we created will still be y = 480 even though it's only 240 pixels down from
 * the top. 
 */
		topLeftViewport.x = 0;
		topLeftViewport.y = 0;
		topLeftViewport.w = SCREEN_WIDTH / 2;
		topLeftViewport.h = SCREEN_HEIGHT / 2;
		SDL_RenderSetViewport(gRenderer, &topLeftViewport);
		SDL_RenderCopy(gRenderer, gTexture, NULL, NULL);
/*
 * Here we define the top right area and draw to it. It's pretty much the same
 * as before, only now the x coordinate is half the screen over.
 */
		topRightViewport.x = SCREEN_WIDTH / 2;
		topRightViewport.y = 0;
		topRightViewport.w = SCREEN_WIDTH / 2;
		topRightViewport.h = SCREEN_HEIGHT / 2;
		SDL_RenderSetViewport(gRenderer, &topRightViewport);
		SDL_RenderCopy(gRenderer, gTexture, NULL, NULL);
/*
 * Finally we render one more time in the bottom half of the screen. Again, the
 * view port will use the same coordinate system as the window it is used in,
 * so the image will appear squished since the viewport is half the height.
 */
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

