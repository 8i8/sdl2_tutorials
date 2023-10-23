/*
 * Atomic Operations
 *
 * Semaphores operate at an operating system level. Atomic operations are a way
 * to lock data at a efficient CPU level. Here we'll be locking a critical
 * section using GPU spinlocks.
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
 * Instead of a semaphore we'll be using a spinlock to protect our data buffer.
 */
SDL_Window* gWindow = NULL;
SDL_Renderer* gRenderer = NULL;
LTexture gSplashTexture;
SDL_SpinLock gDataLock = 0;

int gData = -1;	

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

/*
 * Unlike semaphores, spin locks do not need to be allocated and deallocated.
 */
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
 * Here our critical section is protected by SDL_AtomicLock and SDL_AtomicUnlock.
 *
 * In this case it may seem like semaphores and atomic locks are the same, but
 * remember that semaphores can allow access beyond a single thread. Atomic
 * operations are for when you want a strict locked/unlocked state.
 */
int worker(void* data)
{
	char *str;
	str = data;

	SDL_Log("%s start.", str);

	srand(SDL_GetTicks());
	
	int i;
	for(i = 0; i < 5; ++i) {
		SDL_Delay(16 + rand() % 32);
		SDL_AtomicLock(&gDataLock);	
		SDL_Log("%s gets %d", str, gData);
		gData = rand() % 256;
		SDL_Log("%s sets %d\n\n", str, gData);
		SDL_AtomicUnlock(&gDataLock);	
		SDL_Delay(16 + rand() % 640);
	}

	SDL_Log("%s end.", str);

	return 0;
}

int main(int argc, char* args[])
{
	if(init())
		return -1;

	if(loadMedia())
		return -1;

	SDL_Event e;

	srand(SDL_GetTicks());
	SDL_Thread* threadA = SDL_CreateThread(
					worker, "Thread A", (void*)"Thread A");
	SDL_Delay(16 + rand() % 32);
	SDL_Thread* threadB = SDL_CreateThread(
					worker, "Thread B", (void*)"Thread B");
	
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

	SDL_WaitThread(threadA, NULL);
	SDL_WaitThread(threadB, NULL);
equit:
	close_all();

	return 0;
}

