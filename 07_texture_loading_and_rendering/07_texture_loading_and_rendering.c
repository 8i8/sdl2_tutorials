/*
 * Texture Loading and Rendering
 *
 * A major new addition to SDL 2 is the texture rendering API. This gives you
 * fast, flexible hardware based rendering. In this tutorial we'll be using
 * this new rendering technique.
 */
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>

#define SCREEN_WIDTH	640
#define SCREEN_HEIGHT	480

/*
 * Textures in SDL have their own data type intuitively called a SDL_Texture.
 * When we deal with SDL textures you need an SDL_Renderer to render it to the
 * screen which is why we declare a global renderer named "gRenderer".
 *
 * As you can also see we have a new image loading routine with loadTexture and
 * a globally declared texture we're going to load.
 */
SDL_Texture* loadTexture(char *path);
SDL_Window* gWindow = NULL;
SDL_Renderer* gRenderer = NULL;
SDL_Texture* gTexture = NULL;

/*
 * After we create our window, we have to create a renderer for our window so
 * we can render textures on it. Fortunately this is easily done with a call to
 * SDL_CreateRenderer.
 *
 * After creating the renderer, we want to initialize the rendering color using
 * SDL_SetRenderDrawColor. This controls what color is used for various
 * rendering operations.
 *
 * https://wiki.libsdl.org/SDL_HINT_RENDER_SCALE_QUALITY
 *
 * 	0 or nearest 	→	nearest pixel sampling
 * 	1 or linear	→	linear filtering (supported by OpenGL and Direct3D)
 *	2 or best	→	anisotropic filtering (supported by Direct3D)
 */
short init(void)
{
	if(SDL_Init(SDL_INIT_VIDEO) < 0) {
		SDL_Log("%s(), SDL_Init failed. %s", __func__, SDL_GetError());
		return -1;
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

	if((IMG_Init(IMG_INIT_PNG) & IMG_INIT_PNG) == 0) {
		SDL_Log("%s(), IMG_Init failed. %s", __func__, IMG_GetError());
		return -1;
	}

	return 0;
}

/*
 * Our texture loading function looks largely the same as before only now
 * instead of converting the loaded surface to the display format, we create a
 * texture from the loaded surface using SDL_CreateTextureFromSurface. Like
 * before, this function creates a new texture from an existing surface which
 * means like before we have to free the loaded surface and then return the
 * loaded texture.
 */
SDL_Texture* loadTexture(char *path)
{
	
	SDL_Texture* newTexture = NULL;
	SDL_Surface* loadedSurface;
	
	if((loadedSurface = IMG_Load(path)) == NULL) {
		SDL_Log("%s(), IMG_Load failed. %s", __func__, IMG_GetError());
		return NULL;
	}

	if((newTexture = SDL_CreateTextureFromSurface(
					gRenderer, loadedSurface)) == NULL) {
		SDL_Log("%s(), SDL_CreateTextureFromSurface failed. %s", __func__, SDL_GetError());
		return NULL;
	}

	SDL_FreeSurface(loadedSurface);

	return newTexture;
}

/*
 * Since texture loading is abstracted with our image loading function, the
 * loadMedia() function works pretty much the same as before.
 *
 * In our clean up function, we have to remember to deallocate our textures
 * using SDL_DestroyTexture. 
 */
short loadMedia(void)
{
	if((gTexture = loadTexture((char*)"texture.png")) == NULL)
		return -1;
	return 0;
}

void close_all(void)
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

/*
 * In the main loop after the event loop, we call SDL_RenderClear. This
 * function fills the screen with the color that was last set with
 * SDL_SetRenderDrawColor.
 *
 * With the screen cleared, we render the texture with SDL_RenderCopy. With the
 * texture rendered, we still have to update the screen, but since we're not
 * using SDL_Surfaces to render we can't use SDL_UpdateWindowSurface. Instead
 * we have to use SDL_RenderPresent.
 */
int main(int argc, char* argv[])
{
	SDL_Event e;

	if(init())
		goto equit;

	if(loadMedia())
		goto equit;

	while(1) {
		while(SDL_PollEvent(&e) != 0)
			if(e.type == SDL_QUIT)
				goto equit;
		
		SDL_RenderClear(gRenderer);
		SDL_RenderCopy(gRenderer, gTexture, NULL, NULL);
		SDL_RenderPresent(gRenderer);
		SDL_Delay(60);
	}
equit:
	close_all();

	return 0;
}

