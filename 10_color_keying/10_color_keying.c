/*
 * Color Keying
 *
 * When rendering multiple images on the screen, having images with transparent
 * backgrounds is usually necessary. Fortunately SDL provides an easy way to do
 * this using color keying.
 */
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>

#define SCREEN_WIDTH	640
#define SCREEN_HEIGHT	480

/*
 * For this tutorial we're going to wrap the SDL_Texture in a struct to make
 * some things easier. For example, if you want to get certain information
 * about the texture such as its width or height you would have to use some SDL
 * functions to query the information for the texture. Instead what we're going
 * to do is use a struct to wrap and store the information about the texture.
 */
typedef struct {
	SDL_Texture *mTexture;
	int mWidth;
	int mHeight;
} LTexture;

SDL_Window* gWindow = NULL;
SDL_Renderer* gRenderer = NULL;

/*
 * For this scene there's two textures we're going to load here declared as
 * "gFooTexture" and "gBackgroundTexture". We're going to take this foo'
 * texture, color key the cyan (light blue) colored background and render it on
 * top of this background:
 */
LTexture gFooTexture;
LTexture gBackgroundTexture;

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
 * The function initializes variables and the next calls the
 * deallocator which we'll cover later.
 */
LTexture *LTexture_init(LTexture *lt)
{
	lt->mTexture = NULL;
	lt->mWidth = 0;
	lt->mHeight = 0;
	return lt;
}

/*
 * The deallocator simply checks if a texture exists, destroys it, and
 * reinitializes the member variables.
 */
LTexture *free_texture(LTexture *lt)
{
	if(lt->mTexture != NULL)
	{
		SDL_DestroyTexture(lt->mTexture);
		lt->mTexture = NULL;
		lt->mWidth = 0;
		lt->mHeight = 0;
	}
	return lt;
}

/*
 * The texture loading function pretty much works like it did in the texture
 * loading tutorial but with some small but important tweaks. First off we
 * deallocate the texture in case there's one that's already loaded.
 *
 * ../07_texture_loading_and_rendering/07_texture_loading_and_rendering.c
 */
int loadFromFile(LTexture *lt, char *path)
{
	free_texture(lt);

	SDL_Texture* newTexture = NULL;

	SDL_Surface* loadedSurface = IMG_Load(path);
	if(loadedSurface == NULL) {
		SDL_Log("%s(), IMG_Load failed. %s", __func__, IMG_GetError());
		return -1;
	}

/*
 * Next, we color key the image with SDL_SetColorKey before creating a texture
 * from it. The first argument is the surface we want to color key, the second
 * argument covers whether we want to enable color keying, and the last
 * argument is the pixel we want to color key with.
 * 
 * The most cross platform way to create a pixel from RGB color is with
 * SDL_MapRGB. The first argument is the format we want the pixel in.
 * Fortunately the loaded surface has a format member variable. The last three
 * variables are the red, green, and blue components for color you want to map.
 * Here we're mapping cyan which is red 0, green 255, blue 255.
 */
	SDL_SetColorKey(loadedSurface, SDL_TRUE, SDL_MapRGB(
				loadedSurface->format, 0x00, 0xFF, 0xFF));

	newTexture = SDL_CreateTextureFromSurface(gRenderer, loadedSurface);
	if(newTexture == NULL) {
		SDL_Log("%s(), SDL_CreateTextureFromSurface failed. %s",
				__func__, SDL_GetError());
		return -1;
	}
/*
 * After color keying the loaded surface, we create a texture from the loaded
 * and color keyed surface. If the texture was created successfully, we store
 * the width/height of the texture and return whether the texture loaded
 * success fully.
 */
	lt->mWidth = loadedSurface->w;
	lt->mHeight = loadedSurface->h;

	SDL_FreeSurface(loadedSurface);

	lt->mTexture = newTexture;

	return 0;
}

/*
 * Here you see why we needed a wrapper class. Up until now, we've been pretty
 * much been rendering full screen images so we didn't need to specify
 * position. Because we didn't need to specify position, we just called
 * SDL_RenderCopy with the last two arguments as NULL.
 *
 * When rendering a texture in a certain place, you need to specify a
 * destination rectangle that sets the x/y position and width/height. We can't
 * specify the width/height without knowing the original image's dimensions. So
 * here when we render our texture we create a rectangle with the position
 * arguments and the member width/height, and pass in this rectangle to
 * SDL_RenderCopy.
 */
short render(LTexture *lt, int x, int y)
{
	SDL_Rect renderQuad = {x, y, lt->mWidth, lt->mHeight};
	SDL_RenderCopy(gRenderer, lt->mTexture, NULL, &renderQuad);
	return 0;
}

short loadMedia(void)
{
	if((loadFromFile(&gFooTexture, "foo.png")) < 0)
		return -1;

	if((loadFromFile(&gBackgroundTexture, "background.png")) < 0)
		return -1;

	return 0;
}

void close_all(void)
{
	free_texture(&gFooTexture);
	free_texture(&gBackgroundTexture);

	SDL_DestroyRenderer(gRenderer);
	SDL_DestroyWindow(gWindow);
	gWindow = NULL;
	gRenderer = NULL;

	IMG_Quit();
	SDL_Quit();
}

/*
 * Here is the main loop with our textures rendering. It's a basic loop that
 * handles events, clears the screen, renders the background, renders the stick
 * figure on top of it, and updates the screen.
 *
 * An important thing to note is that order matters when you're rendering
 * multiple things to the screen every frame. If we to render the stick figure
 * first, the background will render over it and you won't be able to see the
 * stick figure.
 */
int main(int argc, char* argv[])
{
	if(init())
		goto equit;

	if(loadMedia())
		goto equit;

	SDL_Event e;

	while(1)
	{
		while(SDL_PollEvent(&e) != 0)
			if(e.type == SDL_QUIT)
				goto equit;

		SDL_SetRenderDrawColor(gRenderer, 0xFF, 0xFF, 0xFF, 0xFF);
		SDL_RenderClear(gRenderer);

		render(&gBackgroundTexture, 0, 0);
		render(&gFooTexture, 240, 190);

		SDL_RenderPresent(gRenderer);
	}
equit:
	close_all();

	return 0;
}

