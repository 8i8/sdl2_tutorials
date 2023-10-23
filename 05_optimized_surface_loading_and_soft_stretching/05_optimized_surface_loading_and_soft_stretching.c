/*
 * Up until now we've been blitting our images raw. Since we were only showing
 * one image, it didn't matter. When you're making a game, blitting images raw
 * causes needless slow down. We'll be converting them to an optimized format
 * to speed them up.
 *
 * SDL 2 also has a new feature for SDL surfaces called soft stretching, which
 * allows you to blit an image scaled to a different size. In this tutorial
 * we'll take an image half the size of the screen and stretch it to the full
 * size.
 *
 * https://wiki.libsdl.org/SDL_BlitScaled
 * https://wiki.libsdl.org/SDL_ConvertSurface
 *
 * Note: SDL_SWSURFACE was added to SDL_ConvertSurface in place of the NULL
 * value that was present on downloading ...
 */
#include <SDL2/SDL.h>

#define SCREEN_WIDTH	640
#define SCREEN_HEIGHT	480

SDL_Surface* loadSurface(char *str);
SDL_Window* gWindow = NULL;
SDL_Surface* gScreenSurface = NULL;
SDL_Surface* gStretchedSurface = NULL;

int init(void)
{
	if(SDL_Init(SDL_INIT_VIDEO) < 0) {
		SDL_Log("%s(), SDL_Init failed. %s", __func__, SDL_GetError());
		return -1;
	}

	gWindow = SDL_CreateWindow(
					"SDL Tutorial +",
					SDL_WINDOWPOS_UNDEFINED,
					SDL_WINDOWPOS_UNDEFINED,
					SCREEN_WIDTH, SCREEN_HEIGHT,
					SDL_WINDOW_SHOWN);
	if(gWindow == NULL) {
		SDL_Log("%s(), SDL_CreateWindow failed. %s", __func__, SDL_GetError());
		return -1;
	}

	gScreenSurface = SDL_GetWindowSurface(gWindow);

	return 0;
}

int loadMedia(void)
{
	if((gStretchedSurface = loadSurface("stretch.bmp")) == NULL)
		return -1;

	return 0;
}

/*
 * Back in our image loading function, we're going to make some modifications
 * so the surface is converted on load. At the top of the function we pretty
 * much load images like we did in previous tutorials, but we also declare a
 * pointer to the final optimized image.
 */
SDL_Surface* loadSurface(char *path)
{
	SDL_Surface* optimizedSurface = NULL;

	SDL_Surface* loadedSurface = SDL_LoadBMP(path);
	if(loadedSurface == NULL) {
		SDL_Log("%s(), SDL_LoadBMP failed. %s", __func__, SDL_GetError());
		return NULL;
	}

/*
 * If the image loaded successfully in the previous lines of code, we optimize
 * the surface we loaded.
 *
 * See when you load a bitmap, it's typically loaded in a 24bit format since
 * most bitmaps are 24bit. Most, if not all, modern displays are not 24bit by
 * default. If we blit an image that's 24bit onto a 32bit image, SDL will
 * convert it every single time the image is blitted.
 *
 * So what we're going to do when an image is loaded is convert it to the same
 * format as the screen so no conversion needs to be done on blit. This can be
 * done easily with SDL_ConvertSurface. All we have to do is pass in the
 * surface want to convert with the format of the screen.
 *
 * It's important to note that SDL_ConvertSurface returns a copy of the
 * original in a new format. The original loaded image is still in memory after
 * this call. This means we have to free the original loaded surface or we'll
 * have two copies of the same image in memory.
 *
 * After the image is loaded and converted, we return the final optimized
 * image.
 *
 * By the way, I keep getting e-mails saying that the last argument
 * SDL_ConvertSurface should be 0. NULL is 0. If your compiler barks a warning
 * at you, you can change that last argument to 0.
 */
	optimizedSurface = SDL_ConvertSurface(
						loadedSurface,
						gScreenSurface->format,
						SDL_SWSURFACE);
	if(optimizedSurface == NULL) {
		SDL_Log("%s(), SDL_ConvertSurface failed. %s", __func__, SDL_GetError());
		return NULL;
	}

	SDL_FreeSurface(loadedSurface);

	return optimizedSurface;
}

void close_all(void)
{
	SDL_FreeSurface(gStretchedSurface);
	gStretchedSurface = NULL;
	SDL_DestroyWindow(gWindow);
	gWindow = NULL;
	SDL_Quit();
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
		while(SDL_PollEvent(&e) != 0)
			if(e.type == SDL_QUIT)
				goto equit;
/*
 * SDL 2 has a new dedicated function to blit images to a different size:
 * SDL_BlitScaled. Like blitting images before, SDL_BlitScaled takes in a
 * source surface to blit onto the destination surface. It also takes in a
 * destination SDL_Rect which defines the position and size of the image you
 * are blitting.
 *
 * So if we want to take an image that's smaller than the screen and make it
 * the size of the screen, you make the destination width/height to be the
 * width/height of the screen.
 */
		SDL_Rect stretchRect;
		stretchRect.x = 10;
		stretchRect.y = 10;
		stretchRect.w = SCREEN_WIDTH-20;
		stretchRect.h = SCREEN_HEIGHT-20;
		SDL_BlitScaled(
				gStretchedSurface,
				NULL,
				gScreenSurface,
				&stretchRect);

		SDL_UpdateWindowSurface(gWindow);
		SDL_Delay(60);
	}
equit:
	close_all();

	return 0;
}

