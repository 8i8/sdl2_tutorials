/*
 * This program demonstrates the use of mutex and contitions, using the
 * following commands:
 *
 * https://wiki.libsdl.org/SDL_LockMutex
 * https://wiki.libsdl.org/SDL_UnlockMutex
 * https://wiki.libsdl.org/SDL_CondSignal
 * https://wiki.libsdl.org/SDL_CondWait
 */
#include <SDL2/SDL.h>
#include <SDL2/SDL_thread.h>
#include <SDL2/SDL_image.h>
#include <stdio.h>

#define SCREEN_WIDTH	640
#define SCREEN_HEIGHT	480
#define SCREEN_FPS	60

typedef struct {
	SDL_Texture* mTexture;
	void* mPixels;
	int mPitch;
	int mWidth;
	int mHeight;
} LTexture;

int producer();
int consumer();
void produce();
void consume();

SDL_Window* gWindow = NULL;
SDL_Renderer* gRenderer = NULL;
LTexture gSplashTexture;
SDL_mutex* gBufferLock = NULL;
SDL_cond* gCanProduce = NULL;
SDL_cond* gCanConsume = NULL;

int gData = -1;

short init()
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
		SDL_Log("%s(), SDL_CreateWindow failed.", __func__);
		return -1;
	}

	gRenderer = SDL_CreateRenderer(
					gWindow,
					-1,
					SDL_RENDERER_ACCELERATED
					| SDL_RENDERER_PRESENTVSYNC);
	if(gRenderer == NULL) {
		SDL_Log("%s(), SDL_CreateRenderer failed.", __func__);
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
		SDL_Log("%s(), IMG_Load failed to load \"%s\".",
				__func__, path);
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
		SDL_Log("%s(), SDL_CreateTextureFromSurface failed.", __func__);
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

short loadMedia()
{
	gBufferLock = SDL_CreateMutex();
			
	gCanProduce = SDL_CreateCond();
	gCanConsume = SDL_CreateCond();

	if(LTexture_loadFromFile(&gSplashTexture, "splash.png"))
		return -1;

	return 0;
}

void close_all()
{
	LTexture_free(&gSplashTexture);

	SDL_DestroyMutex(gBufferLock);
	gBufferLock = NULL;
			
	SDL_DestroyCond(gCanProduce);
	SDL_DestroyCond(gCanConsume);
	gCanProduce = NULL;
	gCanConsume = NULL;

	SDL_DestroyRenderer(gRenderer);
	SDL_DestroyWindow(gWindow);
	gWindow = NULL;
	gRenderer = NULL;

	IMG_Quit();
	SDL_Quit();
}

int producer()
{
	printf("Production started ...\n");
	srand(SDL_GetTicks());
	
	int i;
	for(i = 0; i < 5; ++i){
		SDL_Delay(rand() % 1000);
		produce();
	}

	printf("Production completed.\n");
	
	return 0;

}

int consumer()
{
	printf("Consumption started ...\n");

	srand(SDL_GetTicks());

	int i;
	for(i = 0; i < 5; ++i) {
		SDL_Delay(rand() % 1000);
		consume();
	}
	
	printf("Consumption completed!\n");

	return 0;
}

void produce()
{
	SDL_LockMutex(gBufferLock);
	
	if(gData != -1) {
		printf("Producer encountered full buffer, "
				"waiting for consumer to empty buffer...\n");
		SDL_CondWait(gCanProduce, gBufferLock);
	}

	gData = rand() % 255;
	printf("Produced %d\n", gData);
	
	SDL_UnlockMutex(gBufferLock);
	SDL_CondSignal(gCanConsume);
}

void consume()
{
	SDL_LockMutex(gBufferLock);
	
	if(gData == -1) {
		printf("Consumer encountered empty buffer, "
				"waiting for producer to fill buffer...\n");
		SDL_CondWait(gCanConsume, gBufferLock);
	}

	printf("Consumed %d\n", gData);
	gData = -1;
	
	SDL_UnlockMutex(gBufferLock);
	SDL_CondSignal(gCanProduce);
}

int main(int argc, char* args[])
{
	if(init())
		return -1;

	if(loadMedia())
		return -1;

	SDL_Event e;

	SDL_Thread* producerThread = SDL_CreateThread(producer, "Producer", NULL);
	SDL_Thread* consumerThread = SDL_CreateThread(consumer, "Consumer", NULL);
	
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

	SDL_WaitThread(consumerThread, NULL);
	SDL_WaitThread(producerThread, NULL);
equit:
	close_all();

	return 0;
}
