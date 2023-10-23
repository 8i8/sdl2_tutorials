/*
 * Texture Manipulation
 *
 * To do graphics effects often requires pixel access. In this tutorial we'll
 * be altering an image's pixels to white out the background.
 */
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>

#define SCREEN_WIDTH	640
#define SCREEN_HEIGHT	480

/*
 * Here we're adding new functionality related to the texture struct. We have
 * functions to lock/unlock the texture because in order to access a texture's
 * pixels we have to lock it and once we're done messing with the pixels we
 * have to unlock it.
 *
 * We have a function to get the raw pixels and a function to get the pitch.
 * The pitch is basically the width of the texture in memory. On some older and
 * mobile hardware, there are limitations of what size texture you can have. If
 * you create a texture with a width of 100 pixels, it may get padded to 128
 * pixels wide (the next power of two). Using the pitch, we know how the image
 * is in memory.
 *
 * In terms of data members we have a pointer to the pixels after we lock the
 * texture and the pitch.
 */
typedef struct {
	SDL_Texture* mTexture;
	void* mPixels;
	int mPitch;
	int mWidth;
	int mHeight;
} LTexture;

SDL_Window* gWindow = NULL;
SDL_Renderer* gRenderer = NULL;
LTexture gFooTexture;

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

	gRenderer = SDL_CreateRenderer(
					gWindow,
					-1,
					SDL_RENDERER_ACCELERATED
					| SDL_RENDERER_PRESENTVSYNC);
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

void LTexture_free(LTexture *lt)
{
	if(lt->mTexture != NULL) {
		SDL_DestroyTexture(lt->mTexture);
		lt->mTexture = NULL;
		lt->mWidth = 0;
		lt->mHeight = 0;
		lt->mPixels = NULL;
		lt->mPitch = 0;
	}
}

/*
 * To allow ourselves the ability to edit the texture, we have to load our
 * textures differently. When we created textures from surfaces before, they
 * had the default SDL_TextureAccess of SDL_TEXTUREACCESS_STATIC, which means
 * we can't change it after it is created. In order to be able to edit the
 * texture's pixels we have to create the texture with
 * SDL_TEXTUREACCESS_STREAMING.
 *
 * First we have to load the image as a surface like before. We then have to
 * convert the surface to the same pixel format as the window using
 * SDL_ConvertSurfaceFormat because we can't mix texture rendering and surface
 * rendering calls. We then create a blank texture with SDL_CreateTexture.
 */
short LTexture_loadFromFile(LTexture *lt, char *path)
{
	LTexture_free(lt);

	SDL_Texture* newTexture = NULL;

	SDL_Surface* loadedSurface = IMG_Load(path);
	if(loadedSurface == NULL) {
		SDL_Log("%s(), IMG_Load failed. %s", __func__, IMG_GetError());
		return -1;
	}

	SDL_Surface* formattedSurface = SDL_ConvertSurfaceFormat(
						loadedSurface,
						SDL_GetWindowPixelFormat(gWindow),
						SDL_SWSURFACE);
	if(formattedSurface == NULL) {
		SDL_Log("%s(), SDL_ConvertSurfaceFormat failed.", __func__);
		return -1;
	}

	newTexture = SDL_CreateTexture(
						gRenderer,
						SDL_GetWindowPixelFormat(gWindow),
						SDL_TEXTUREACCESS_STREAMING,
						formattedSurface->w,
						formattedSurface->h);
	if(newTexture == NULL) {
		SDL_Log("%s(), SDL_CreateTextureFromSurface failed. %s", __func__, SDL_GetError());
		return -1;
	}
/*
 * After the texture is created we have to manually copy the pixels from the
 * surface to the texture. To grab the pixels from the texture we use
 * SDL_LockTexture. The first argument is the texture we'll be grabbing pixels
 * from. The second argument is the region we want to grab pixels from and
 * since we're getting the pixels from the whole texture we set this argument
 * to NULL. The third argument is the pointer that will be set to the address
 * of the pixels and the last argument will be set the the texture's pitch.
 *
 * After we have the texture's pixels, we copy the pixels from the surface to
 * the texture using memcpy. The first argument is the destination, the second
 * argument is the source, and the third argument is the number of bytes we'll
 * be copying. Fortunately, the pitch SDL gives us is the number of bytes per
 * row of pixels so all we have to do is multiply by the height of the surface
 * to copy in all the pixels from the image.
 *
 * After we're done copying the pixels from the surface to the texture, we
 * unlock the texture to update it with the new pixels using SDL_UnlockTexture.
 * After the texture is unlocked the pixel pointer is invalid so we set it to
 * NULL.
 *
 * With the pixels from the surface copied into the texture, we then get rid of
 * the old surfaces and return 0 if the texture loaded successfully.
 */
	SDL_LockTexture(newTexture, NULL, &lt->mPixels, &lt->mPitch);

	memcpy(
			lt->mPixels,
			formattedSurface->pixels,
			formattedSurface->pitch * formattedSurface->h);

	SDL_UnlockTexture(newTexture);
	lt->mPixels = NULL;

	lt->mWidth = formattedSurface->w;
	lt->mHeight = formattedSurface->h;

	SDL_FreeSurface(formattedSurface);
		
	SDL_FreeSurface(loadedSurface);

	lt->mTexture = newTexture;

	return 0;
}

void LTexture_render(
			LTexture *lt,
			int x, int y,
			SDL_Rect* clip,
			double angle,
			SDL_Point* center,
			SDL_RendererFlip flip)
{
	SDL_Rect renderQuad = { x, y, lt->mWidth, lt->mHeight };

	if(clip != NULL) {
		renderQuad.w = clip->w;
		renderQuad.h = clip->h;
	}

	SDL_RenderCopyEx(gRenderer, lt->mTexture, clip,
				&renderQuad, angle, center, flip);
}

int LTexture_getWidth(LTexture *lt)
{
	return lt->mWidth;
}

int LTexture_getHeight(LTexture *lt)
{
	return lt->mHeight;
}

/*
 * Here are our functions to lock/unlock the texture after loading the image. 
 */
short LTexture_lockTexture(LTexture *lt)
{
	if(lt->mPixels != NULL) {
		SDL_Log("Texture already locked.");
		return -1;
	}

	if(SDL_LockTexture(lt->mTexture, NULL, &lt->mPixels, &lt->mPitch) != 0) {
		SDL_Log("%s(), SDL_LockTexture failed. %s",
				__func__, SDL_GetError());
		return -1;
	}
	return 0;
}

short LTexture_unlockTexture(LTexture *lt)
{
	if(lt->mPixels == NULL) {
		SDL_Log("Texture not locked.");
		return -1;
	}

	SDL_UnlockTexture(lt->mTexture);
	lt->mPixels = NULL;
	lt->mPitch = 0;

	return 0;
}

/*
 * Finally here are the accessors to get the pixels and pitch while the texture
 * is locked. Now that we can create a streamable texture and lock/unlock it,
 * it is time to do some texture pixel processing.
 */
void* LTexture_getPixels(LTexture *lt)
{
	return lt->mPixels;
}

int LTexture_getPitch(LTexture *lt)
{
	return lt->mPitch;
}

/*
 * In our media loading function after we load the texture we lock it so we can
 * alter its pixels.
 *
 * After the texture is locked, we're going to go through the pixels and make
 * all the background pixels transparent. What we're doing is essentially
 * manually color keying the image.
 *
 * First we allocate a pixel format using SDL_GetWindowPixelFormat and
 * SDL_AllocFormat. We then need to grab the pixels. Our pixel accessor returns
 * a void pointer and we want 32bit pixels so we type cast it to a 32bit
 * unsigned integer.
 *
 */
short loadMedia(void)
{
	if(LTexture_loadFromFile(&gFooTexture, "foo.png"))
		return -1;

	if(LTexture_lockTexture(&gFooTexture))
		return -1;

	Uint32 format = SDL_GetWindowPixelFormat(gWindow);
	SDL_PixelFormat* mappingFormat = SDL_AllocFormat(format);
/*
 * Next we want to get the number of pixels. We get the pitch which is the
 * width in bytes. We need the width in pixels and since there are 4 bytes per
 * pixel all we need to do is divide by 4 to get the pitch in pixels. Then we
 * multiply the pitch width by the height to get the total number of pixels.
 */
	Uint32* pixels = (Uint32*)LTexture_getPixels(&gFooTexture);
	int pixelCount = (LTexture_getPitch(&gFooTexture) / 4)
					* LTexture_getHeight(&gFooTexture);
/*
 * What we're going to do is find all the pixels that the color key color and
 * then replace them with transparent pixels. First we map color key color and
 * the transparent color using the window's pixel format. Then we go through
 * all the pixels and check if any of the pixels match the color key. If it
 * does, we give the value of a transparent pixel.
 */
	Uint32 colorKey = SDL_MapRGB(mappingFormat, 0, 0xFF, 0xFF);
	Uint32 transparent = SDL_MapRGBA(mappingFormat, 0xFF, 0xFF, 0xFF, 0x00);
	int i;
	for(i = 0; i < pixelCount; ++i)
		if(pixels[i] == colorKey)
			pixels[i] = transparent;
/*
 * After we're done going through the pixels we unlock the texture to update it
 * with the new pixels. Lastly we can't forget to call SDL_FreeFormat to
 * deallocate the pixel format we created.
 */
	LTexture_unlockTexture(&gFooTexture);
	SDL_FreeFormat(mappingFormat);

	return 0;
}

void close_all(void)
{
	LTexture_free(&gFooTexture);

	SDL_DestroyRenderer(gRenderer);
	gRenderer = NULL;
	SDL_DestroyWindow(gWindow);
	gWindow = NULL;

	IMG_Quit();
	SDL_Quit();
}

int main(int argc, char* args[])
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

		LTexture_render(
				&gFooTexture,
				(SCREEN_WIDTH - LTexture_getWidth(&gFooTexture)) / 2,
				(SCREEN_HEIGHT - LTexture_getHeight(&gFooTexture)) / 2,
				NULL, 0.0, NULL, 0);

		SDL_RenderPresent(gRenderer);
	}
equit:
	close_all();

	return 0;
}
