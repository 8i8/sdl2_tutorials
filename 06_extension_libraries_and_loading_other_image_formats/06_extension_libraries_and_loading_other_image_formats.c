/*
 * Extension Libraries and Loading Other Image Formats
 * 
 * SDL extension libraries allow you do things like load image files besides
 * BMP, render TTF fonts, and play music. You can set up SDL_image to load PNG
 * files, which can save you a lot of disk space. In this tutorial we'll be
 * covering how to install SDL_image.
 */
/*
 * SDL itself is an extension library since it adds game and media
 * functionality that doesn't come standard with your C++ compiler. As you're
 * setting up your extension library, you'll realize it's nearly identical to
 * installing SDL by itself. We'll be specifically installing SDL_image, but if
 * you can install that extension library you should be able to install any of
 * them.
 *
 * After you set up SDL_image, we'll cover how to create load a PNG with SDL.
 *
 * https://www.libsdl.org/projects/SDL_image/docs/SDL_image_8.html
 */
/*
 * Loading PNGs with SDL_image
 *
 * Now that the library is all set up, let's load some PNGs.
 */
/*
 * To use any SDL_image function or data types, we need to include the
 * SDL_image header. We'd have to do the same for SDL_ttf, or SDL_mixer.
 */
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>

#define SCREEN_WIDTH	640
#define SCREEN_HEIGHT	480

SDL_Surface* loadSurface(char *path);
SDL_Window* gWindow = NULL;
SDL_Surface* gScreenSurface = NULL;
SDL_Surface* gPNGSurface = NULL;

/*
 * Now that we're using SDL_image, we need to initialize it. Here we want to
 * initialize SDL_image with PNG loading, so we pass in the PNG loading flags
 * into IMG_Init. IMG_Init returns the flags that loaded successfully. If the
 * flags that are returned do not contain the flags we requested, that means
 * there's an error.
 *
 * When there's an error with SDL_image, you get error string with IMG_GetError
 * as opposed to SDL_GetError.
 */
short init(void)
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
		SDL_Log("%s(), SDL_CreateWindow failed. %s", __func__, SDL_GetError());
		return -1;
	}

	if((IMG_Init(IMG_INIT_PNG)& IMG_INIT_PNG) == 0) {
		SDL_Log("%s(), IMG_Init failed. %s", __func__, IMG_GetError());
		return -1;
	}

	gScreenSurface = SDL_GetWindowSurface(gWindow);

	return 0;
}

short loadMedia(void)
{
	if((gPNGSurface = loadSurface("loaded.png")) == NULL)
		return -1;

	return 0;
}

void close_all(void)
{
	SDL_FreeSurface(gPNGSurface);
	gPNGSurface = NULL;
	SDL_DestroyWindow(gWindow);
	gWindow = NULL;
	IMG_Quit();
	SDL_Quit();
}

/*
 * Our image loading function is pretty much the same as before, only now it
 * uses IMG_Load as opposed to SDL_LoadBMP. IMG_Load can load many different
 * types of format which you can find out about in the SDL_image documentation.
 * Like with IMG_Init, when there's an error with IMG_Load, we call
 * IMG_GetError to get the error string.
 */
SDL_Surface* loadSurface(char *path)
{
	
	SDL_Surface* optimizedSurface = NULL;
	SDL_Surface* loadedSurface = NULL;

	if((loadedSurface = IMG_Load(path)) == NULL) {
		SDL_Log("%s(), IMG_Load failed. %s", __func__, IMG_GetError());
		return NULL;
	}

	optimizedSurface = SDL_ConvertSurface(
						loadedSurface,
						gScreenSurface->format,
						SDL_SWSURFACE);
	if(optimizedSurface == NULL) {
		SDL_Log("%s(), SDL_ConvertSurface failed %s.", __func__, SDL_GetError());
		return NULL;
	}

	SDL_FreeSurface(loadedSurface);

	return optimizedSurface;
}

int main(int argc, char* args[])
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

		SDL_BlitSurface(gPNGSurface, NULL, gScreenSurface, NULL);
		SDL_UpdateWindowSurface(gWindow);
		SDL_Delay(60);
	}
equit:
	close_all();

	return 0;
}

