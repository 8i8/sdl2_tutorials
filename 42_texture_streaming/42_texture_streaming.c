/*
 * Texture Streaming
 *
 * Sometime we want to render pixel data from a source other than a bitmap like
 * a web cam. Using texture stream we can render pixels from any source.
 */
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>

#define IMG_NUM		4
#define FRAME_DELAY	4
#define SCREEN_WIDTH	640
#define SCREEN_HEIGHT	480

/*
 * Here we're add more functionality to our texture struct with more related
 * functions. The createBlank function allocates a blank texture that we can
 * copy data to when streaming. The copyPixels function copies in the pixel
 * data we want to stream.
 */
typedef struct {
	SDL_Texture* mTexture;
	void* mPixels;
	int mPitch;
	int mWidth;
	int mHeight;
} LTexture;

/*
 * Here is our data stream class. We won't go deep into how it works because we
 * don't really care. When dealing with web cam, video decoding, etc APIs they
 * typically don't go deep into how they work because all we care about is
 * getting the video and audio data from them.
 *
 * All we really care about is that getBuffer function which gets the current
 * pixels from the data buffer.
 */
typedef struct {
	SDL_Surface* mImages[IMG_NUM];
	int mCurrentImage;
	int mDelayFrames;
} DataStream;

SDL_Window* gWindow = NULL;
SDL_Renderer* gRenderer = NULL;
LTexture gStreamingTexture;
DataStream gDataStream;

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
 * As you can see, all this function does is create a 32bit RGBA texture with
 * stream access. One thing you have to make sure of when creating your texture
 * is that the format of the texture pixels matches the format of the pixels
 * we're streaming. 
 */
short LTexture_createBlank(LTexture *lt, int width, int height)
{
	lt->mTexture = SDL_CreateTexture(
					gRenderer,
					SDL_PIXELFORMAT_RGBA8888,
					SDL_TEXTUREACCESS_STREAMING,
					width,
					height);
	if(lt->mTexture == NULL) {
		SDL_Log("%s(), SDL_CreateTexture failed.", __func__);
		return -1;
	}

	lt->mWidth = width;
	lt->mHeight = height;

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
 * Here is our function to copy in the pixels from the stream. The function
 * assumes the texture is locked and that the pixels are from an image the same
 * size as the texture.
 */
void LTexture_copyPixels(LTexture *lt, void* pixels)
{
	if(lt->mPixels != NULL)
		memcpy(lt->mPixels, pixels, lt->mPitch * lt->mHeight);
}

void DataStream_init(DataStream *ds)
{
	int i;
	for (i = 0; i < IMG_NUM; i++)
		ds->mImages[i] = NULL;

	ds->mCurrentImage = 0;
	ds->mDelayFrames = FRAME_DELAY;
}

short DataStream_loadMedia(DataStream *ds)
{
	int i;
	char path[64] = "";
	SDL_Surface* loadedSurface;

	for(i = 0; i < IMG_NUM; i++) {
		sprintf(path, "foo_walk_%d.png", i);
		if((loadedSurface = IMG_Load(path)) == NULL) {
			SDL_Log("%s(), IMG_Load failed. %s", __func__, IMG_GetError());
			return -1;
		}
		ds->mImages[i] = SDL_ConvertSurfaceFormat(
						loadedSurface,
						SDL_PIXELFORMAT_RGBA8888,
						SDL_SWSURFACE);
		SDL_FreeSurface(loadedSurface);
	}
	return 0;
}

void DataStream_free(DataStream *ds)
{
	int i;
	for(i = 0; i < IMG_NUM; ++i)
		SDL_FreeSurface(ds->mImages[i]);
}

void* DataStream_getBuffer(DataStream *ds)
{
	--(ds->mDelayFrames);

	if(ds->mDelayFrames == 0) {
		++(ds->mCurrentImage);
		ds->mDelayFrames = FRAME_DELAY;
	}

	if(ds->mCurrentImage == IMG_NUM)
		ds->mCurrentImage = 0;

	return ds->mImages[ds->mCurrentImage]->pixels;
}

short loadMedia(void)
{
	if(LTexture_createBlank(&gStreamingTexture, 64, 205))
		return -1;

	DataStream_init(&gDataStream);

	if(DataStream_loadMedia(&gDataStream))
		return -1;

	return 0;
}

void close_all(void)
{
	LTexture_free(&gStreamingTexture);
	DataStream_free(&gDataStream);

	SDL_DestroyRenderer(gRenderer);
	SDL_DestroyWindow(gWindow);
	gWindow = NULL;
	gRenderer = NULL;

	IMG_Quit();
	SDL_Quit();
}

/*
 * In the main loop rendering we lock our stream texture, copy the pixels from
 * the stream and then unlock the texture. With our texture holding the latest
 * image from the stream, we render the image to the screen.
 *
 * When dealing with decoding APIs things may get trickier where we have to
 * convert from one format to another but ultimately all we need is a means to
 * get the pixel data and copy it to the screen.
 */
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

		LTexture_lockTexture(&gStreamingTexture);
		LTexture_copyPixels(
				&gStreamingTexture,
				DataStream_getBuffer(&gDataStream));
		LTexture_unlockTexture(&gStreamingTexture);

		LTexture_render(
			&gStreamingTexture,
			(SCREEN_WIDTH - LTexture_getWidth(&gStreamingTexture)) / 2,
			(SCREEN_HEIGHT - LTexture_getHeight(&gStreamingTexture)) / 2,
			NULL, 0.0, NULL, 0);

		SDL_RenderPresent(gRenderer);
	}
equit:
	close_all();

	return 0;
}
