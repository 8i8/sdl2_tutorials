/*
 * Semaphores
 *
 * The only multithreading we've done had the main thread and a second thread
 * each do their own thing. In most cases two threads will have to share data
 * and with semaphores you can prevent two threads from accidentally accessing
 * the same piece of data at once.
 */
#include <SDL2/SDL.h>
#include <SDL2/SDL_thread.h>
#include <SDL2/SDL_image.h>
#include <stdio.h>

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
 * Here is our worker thread function. We will spawn two threads that will each
 * execute their copy of this code.
 *
 * The object gDataLock is our semaphore which will lock our gData buffer. A
 * single integer is not much of a data buffer to protect, but since there are
 * going to be two threads that are going to be reading and writing to it we
 * need to make sure it is only being accessed by one thread at a time.
 */
int worker(void *data);
SDL_Window* gWindow = NULL;
SDL_Renderer* gRenderer = NULL;
LTexture gSplashTexture;
SDL_sem* gDataLock = NULL;
int gData = -1;	

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

/*
 * To create a semaphore we call SDL_CreateSemaphore with an initial value for
 * the semaphore. The initial value controls how many times code can pass
 * through a semaphore before it locks.
 *
 * For example, say you only want 4 threads to run at a time because you're on
 * hardware with 4 cores. You'd give the semaphore a value of 4 to start with
 * to make sure no more than 4 threads run at the same time. In this demo we
 * only want 1 thread accessing the data buffer at once so the mutex starts
 * with a value of one.
 */
short loadMedia(void)
{
	gDataLock = SDL_CreateSemaphore(1);

	if(LTexture_loadFromFile(&gSplashTexture, "splash.png"))
		return -1;

	return 0;
}

/*
 * When we're done with a semaphore we call SDL_DestroySemaphore.
 */
void close_all(void)
{
	LTexture_free(&gSplashTexture);

	SDL_DestroySemaphore(gDataLock);
	gDataLock = NULL;

	SDL_DestroyRenderer(gRenderer);
	SDL_DestroyWindow(gWindow);
	gWindow = NULL;
	gRenderer = NULL;

	IMG_Quit();
	SDL_Quit();
}

/*
 * Here we are starting our worker thread. An important thing to know is that
 * seeding your random value is done per thread, so make sure you seed your
 * random values for each thread you run.
 *
 * What each worker thread does is delay for a semi random amount, print the
 * data that is there when it started working, assign a random number to it,
 * print the number assigned to the data buffer, and delay for a bit more
 * before working again. The reason we need to lock data is because we do not
 * want two threads reading or writing our shared data at the same time.
 *
 * Notice the calls to SDL_SemWait and SDL_SemPost. What's in between them is
 * the critical section or the code we only want one thread to access at once.
 * SDL_SemWait decrements the semaphore count and since the initial value is
 * one, it will lock. After the critical section executes, we call SDL_SemPost
 * to increment the semaphore and unlock it.
 *
 * If we have a situation where thread A locks and then thread B tries to lock,
 * thread B will wait until thread A finishes the critical section and unlocks
 * the semaphore. With the critical section protected by a semaphore
 * lock/unlock pair, only one thread can execute the critical section at once.
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
		SDL_SemWait(gDataLock);	
		SDL_Log("%s gets %d", str, gData);
		gData = rand() % 256;
		SDL_Log("%s sets %d\n", str, gData);
		SDL_SemPost(gDataLock);	
		SDL_Delay(16 + rand() % 640);
	}

	SDL_Log("%s end.", str);

	return 0;
}

/*
 * In the main function before we enter the main loop we launch two worker
 * threads with a bit of random delay in between them. There no guarantee
 * thread A or B will work first but since the data they share is protected, we
 * know they won't try to execute the same piece of code at once.
 *
 * Here the main thread runs while the threads to their work. If the main loop
 * end before the threads finish working, we wait on them to finish with
 * SDL_WaitThread.
 */
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

