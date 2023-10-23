/*
 * Multithreading
 *
 * Multithreading can be used to make your program execute two things at once
 * and take advantage of multithreaded architectures. Here we'll make a simple
 * program that outputs to the console while the main thread runs.* This
 * program demonstrates multithreading using SDL_Thread and SDL_CreateThread.
 *
 * There is a saying in computer science; Premature optimization is the root of
 * all evil
 *
 * A major problem with newbie programmers is that they want to be like the
 * professionals without paying their dues. They hear about a technology that
 * the latest and greatest developers out there are using and they think if the
 * use it too it will make them magically better.
 * 
 * One of these tools is multithreading. Since multicore processors launched at
 * a consumer level in the early 00s, developers have been using this new tech
 * to squeeze out as much performance as they can from their applications.
 * 
 * Here's the important part: a poorly made multithreaded program can perform
 * worse than single thread program. Much worse. The fact is that
 * multithreading inherently adds more overhead because threads then have to be
 * managed. If you do not know the costs of using different multithreading
 * tools, you can end up with code that is much slower than its single threaded
 * equivalent.
 * 
 * The general rule is if you don't know:
 * 
 *     What cache coherency is.
 *     What cache alignment is.
 *     How operating systems handle threads and processes.
 *     How to use a profiler.
 * 
 * You should not be trying to use multithreaded optimization. Play with fire
 * and you will get burned. However doing something not for the sake of
 * performance like asynchronous file loading isn't a bad idea for intermediate
 * game developers.
 */
#include <SDL2/SDL.h>
#include <SDL2/SDL_thread.h>
#include <SDL2/SDL_image.h>

#define SCREEN_WIDTH	640
#define SCREEN_HEIGHT	480

typedef struct {
	SDL_Texture* mTexture;
	void* mPixels;
	int mPitch;
	int mWidth;
	int mHeight;
} LTexture;

/*
 * Just like with callback functions, thread functions need to be declared a
 * certain way. They need to take in a void pointer as an argument and return
 * an integer.
 */
int threadFunction(void* data);

SDL_Window* gWindow = NULL;
SDL_Renderer* gRenderer = NULL;
LTexture gSplashTexture;

short init(void)
{
	if(SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER) < 0) {
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
					SDL_PIXELFORMAT_RGBA8888,
					SDL_SWSURFACE);
	if(formattedSurface == NULL) {
		SDL_Log("%s(), SDL_ConvertSurfaceFormat failed.", __func__);
		return -1;
	}

	newTexture = SDL_CreateTexture(
					gRenderer,
					SDL_PIXELFORMAT_RGBA8888,
					SDL_TEXTUREACCESS_STREAMING,
					formattedSurface->w,
					formattedSurface->h);
	if(newTexture == NULL) {
		SDL_Log("%s(), SDL_CreateTextureFromSurface failed. %s", __func__, SDL_GetError());
		return -1;
	}

	SDL_SetTextureBlendMode(newTexture, SDL_BLENDMODE_BLEND);

	SDL_LockTexture(
				newTexture,
				&formattedSurface->clip_rect,
				&lt->mPixels,
				&lt->mPitch);

	memcpy(
			lt->mPixels,
			formattedSurface->pixels,
			formattedSurface->pitch * formattedSurface->h);

	lt->mWidth = formattedSurface->w;
	lt->mHeight = formattedSurface->h;

	Uint32* pixels = (Uint32*)lt->mPixels;
	int pixelCount = (lt->mPitch / 4) * lt->mHeight;

	Uint32 colorKey = SDL_MapRGB(formattedSurface->format, 0, 0xFF, 0xFF);
	Uint32 transparent = SDL_MapRGBA(
			formattedSurface->format, 0x00, 0xFF, 0xFF, 0x00);

	int i;
	for(i = 0; i < pixelCount; ++i)
		if(pixels[i] == colorKey)
			pixels[i] = transparent;

	SDL_UnlockTexture(newTexture);
	lt->mPixels = NULL;

	SDL_FreeSurface(formattedSurface);
		
	SDL_FreeSurface(loadedSurface);

	lt->mTexture = newTexture;

	return 0;
}

void LTexture_render(
			LTexture *lt,
			int x, int y,
			SDL_Rect* clip)
{
	SDL_Rect renderQuad = { x, y, lt->mWidth, lt->mHeight };

	if(clip != NULL) {
		renderQuad.w = clip->w;
		renderQuad.h = clip->h;
	}

	SDL_RenderCopy(gRenderer, lt->mTexture, clip, &renderQuad);
}

short loadMedia(void)
{
	if(LTexture_loadFromFile(&gSplashTexture, "splash.png"))
		return -1;
	return 0;
}

void close_all(void)
{
	LTexture_free(&gSplashTexture);

	SDL_DestroyRenderer(gRenderer);
	SDL_DestroyWindow(gWindow);
	gWindow = NULL;
	gRenderer = NULL;

	IMG_Quit();
	SDL_Quit();
}

/*
 * Our thread function is fairly simple. All it does is take in the data as an
 * integer and uses it to print a message to the console.
 */
int threadFunction(void* data)
{
	int *i = (int*)data;
	SDL_Log("Running thread with value = %d", *i);

	return 0;
}

/*
 * Before we enter the main loop we run the thread function using
 * SDL_CreateThread. This call will run the function in first argument, give it
 * the name in the second argument (names are used to identify it for debugging
 * purposes), and passes in the data from the third argument.
 *
 * The thread will then execute while the main thread is still going. In case
 * the main loop ends before the thread finishes, we make a call to
 * SDL_WaitThread to make sure the thread finishes before the application
 * closes.
 */
int main(int argc, char* args[])
{
	if(init())
		return 1;

	if(loadMedia())
		return 1;

	int data = 101;
	SDL_Thread* threadID = SDL_CreateThread(
						threadFunction,
						"LazyThread",
						(void*)&data);
	SDL_Event e;

	while(1)
	{
		while(SDL_PollEvent(&e) != 0)
			if(e.type == SDL_QUIT)
				goto equit;
		SDL_SetRenderDrawColor(gRenderer, 0xFF, 0xFF, 0xFF, 0xFF);
		SDL_RenderClear(gRenderer);

		LTexture_render(&gSplashTexture, 0, 0, NULL);

		SDL_RenderPresent(gRenderer);
	}

	SDL_WaitThread(threadID, NULL);
equit:
	close_all();

	return 0;
}

