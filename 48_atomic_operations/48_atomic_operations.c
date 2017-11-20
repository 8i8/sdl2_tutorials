/*This source code copyrighted by Lazy Foo' Productions (2004-2015)
and may not be redistributed without written permission.*/

//Using SDL, SDL Threads, SDL_image, standard IO, and, strings
#include <SDL2/SDL.h>
#include <SDL2/SDL_thread.h>
#include <SDL2/SDL_image.h>
#include <stdio.h>

//Screen dimension constants
const int SCREEN_WIDTH = 640;
const int SCREEN_HEIGHT = 480;

typedef struct {
	SDL_Texture* mTexture;
	void* mPixels;
	int mPitch;
	int mWidth;
	int mHeight;
} LTexture;

SDL_Window* gWindow = NULL;		//The window we'll be rendering to
SDL_Renderer* gRenderer = NULL;		//The window renderer
LTexture gSplashTexture;		//Scene textures
SDL_SpinLock gDataLock = 0;		//Data access spin lock

int gData = -1;				//The "data buffer"

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
	//Get rid of preexisting texture
	LTexture_free(lt);

	//The final texture
	SDL_Texture* newTexture = NULL;

	//Load image at specified path
	SDL_Surface* loadedSurface = IMG_Load(path);
	if(loadedSurface == NULL) {
		printf("Unable to load image %s! SDL_image Error: %s\n",
				path, IMG_GetError());
		return -1;
	}

	//Convert surface to display format
	SDL_Surface* formattedSurface = SDL_ConvertSurfaceFormat(
					loadedSurface,
					SDL_PIXELFORMAT_RGBA8888,
					SDL_SWSURFACE);
	if(formattedSurface == NULL) {
		printf("Unable to convert loaded surface to display format! %s\n",
				SDL_GetError());
		return -1;
	}

	//Create blank streamable texture
	newTexture = SDL_CreateTexture(
					gRenderer,
					SDL_PIXELFORMAT_RGBA8888,
					SDL_TEXTUREACCESS_STREAMING,
					formattedSurface->w,
					formattedSurface->h);
	if(newTexture == NULL) {
		printf("Unable to create blank texture! SDL Error: %s\n",
				SDL_GetError());
		return -1;
	}

	//Enable blending on texture
	SDL_SetTextureBlendMode(newTexture, SDL_BLENDMODE_BLEND);

	//Lock texture for manipulation
	SDL_LockTexture(
			newTexture,
			&formattedSurface->clip_rect,
			&lt->mPixels,
			&lt->mPitch);

	//Copy loaded/formatted surface pixels
	memcpy(
			lt->mPixels,
			formattedSurface->pixels,
			formattedSurface->pitch * formattedSurface->h);

	//Get image dimensions
	lt->mWidth = formattedSurface->w;
	lt->mHeight = formattedSurface->h;

	//Get pixel data in editable format
	Uint32* pixels = (Uint32*)lt->mPixels;
	int pixelCount = (lt->mPitch / 4) * lt->mHeight;

	//Map colors				
	Uint32 colorKey = SDL_MapRGB(formattedSurface->format, 0, 0xFF, 0xFF);
	Uint32 transparent = SDL_MapRGBA(
			formattedSurface->format, 0x00, 0xFF, 0xFF, 0x00);

	//Color key pixels
	int i;
	for(i = 0; i < pixelCount; ++i)
		if(pixels[i] == colorKey)
			pixels[i] = transparent;

	//Unlock texture to update
	SDL_UnlockTexture(newTexture);
	lt->mPixels = NULL;

	//Get rid of old formatted surface
	SDL_FreeSurface(formattedSurface);
		
	//Get rid of old loaded surface
	SDL_FreeSurface(loadedSurface);

	//Return success
	lt->mTexture = newTexture;

	return 0;
}

#ifdef _SDL_TTF_H
short LTexture_loadFromRenderedText(
					LTexture *lt,
					char *textureText,
					SDL_Color textColor)
{
	//Get rid of preexisting texture
	Ltexture_free(lt);

	//Render text surface
	SDL_Surface* textSurface = TTF_RenderText_Solid(
							gFont,
							textureText,
							textColor);
	if(textSurface != NULL) {
		printf("Unable to render text surface! SDL_ttf Error: %s\n",
				TTF_GetError());
		return -1;
	}

	//Create texture from surface pixels
	lt->mTexture = SDL_CreateTextureFromSurface(gRenderer, textSurface);
	if(lt->mTexture == NULL) {
		printf("Unable to create texture from rendered text! SDL Error: %s\n",
				SDL_GetError());
		return -1;
	}

	lt->mWidth = textSurface->w;
	lt->mHeight = textSurface->h;

	SDL_FreeSurface(textSurface);
	
	return 0;
}
#endif
		
short LTexture_createBlank(
				LTexture *lt,
				int width, int height,
				SDL_TextureAccess access)
{
	//Create uninitialized texture
	lt->mTexture = SDL_CreateTexture(
					gRenderer,
					SDL_PIXELFORMAT_RGBA8888,
					access, width, height);
	if(lt->mTexture == NULL) {
		printf("Unable to create blank texture! SDL Error: %s\n",
				SDL_GetError());
		return -1;
	}

	lt->mWidth = width;
	lt->mHeight = height;

	return 0;
}

void LTexture_setColor(LTexture *lt, Uint8 red, Uint8 green, Uint8 blue)
{
	//Modulate texture rgb
	SDL_SetTextureColorMod(lt->mTexture, red, green, blue);
}

void LTexture_setBlendMode(LTexture *lt, SDL_BlendMode blending)
{
	//Set blending function
	SDL_SetTextureBlendMode(lt->mTexture, blending);
}
		
void LTexture_setAlpha(LTexture *lt, Uint8 alpha)
{
	//Modulate texture alpha
	SDL_SetTextureAlphaMod(lt->mTexture, alpha);
}

void LTexture_render(
			LTexture *lt,
			int x, int y,
			SDL_Rect* clip)
{
	//Set rendering space and render to screen
	SDL_Rect renderQuad = { x, y, lt->mWidth, lt->mHeight };

	//Set clip rendering dimensions
	if(clip != NULL) {
		renderQuad.w = clip->w;
		renderQuad.h = clip->h;
	}

	//Render to screen
	SDL_RenderCopy(gRenderer, lt->mTexture, clip, &renderQuad);
}

void LTexture_setAsRenderTarget(LTexture *lt)
{
	//Make self render target
	SDL_SetRenderTarget(gRenderer, lt->mTexture);
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
		printf("Texture is already locked!\n");
		return -1;
	}

	if(SDL_LockTexture(lt->mTexture, NULL, &lt->mPixels, &lt->mPitch) != 0) {
		printf("Unable to lock texture! %s\n", SDL_GetError());
		return -1;
	}
	return 0;
}

short LTexture_unlockTexture(LTexture *lt)
{
	if(lt->mPixels == NULL) {
		printf("Texture is not locked!\n");
		return -1;
	}

	SDL_UnlockTexture(lt->mTexture);
	lt->mPixels = NULL;
	lt->mPitch = 0;

	return 0;
}

void* LTexture_getPixels(LTexture *lt)
{
	return lt->mPixels;
}

void LTexture_copyPixels(LTexture *lt, void* pixels)
{
	if(lt->mPixels != NULL)
		memcpy(lt->mPixels, pixels, lt->mPitch * lt->mHeight);
}

int LTexture_getPitch(LTexture *lt)
{
	return lt->mPitch;
}

Uint32 LTexture_getPixel32(LTexture *lt, unsigned int x, unsigned int y)
{
	Uint32 *pixels = (Uint32*)lt->mPixels;

	return pixels[(y * (lt->mPitch / 4)) + x];
}

short init()
{
	//Initialize SDL
	if(SDL_Init(SDL_INIT_VIDEO) < 0) {
		printf("SDL could not initialize! SDL Error: %s\n",
				SDL_GetError());
		return -1;
	}

	//Set texture filtering to linear
	if(SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "1") == 0)
		printf("Warning: Linear texture filtering not enabled!");

	//Create window
	gWindow = SDL_CreateWindow(
					"SDL Tutorial",
					SDL_WINDOWPOS_UNDEFINED,
					SDL_WINDOWPOS_UNDEFINED,
					SCREEN_WIDTH,
					SCREEN_HEIGHT,
					SDL_WINDOW_SHOWN);
	if(gWindow == NULL) {
		printf("Window could not be created! SDL Error: %s\n",
				SDL_GetError());
		return -1;
	}

	//Create renderer for window
	gRenderer = SDL_CreateRenderer(
					gWindow,
					-1,
					SDL_RENDERER_ACCELERATED
					| SDL_RENDERER_PRESENTVSYNC);
	if(gRenderer == NULL) {
		printf("Renderer could not be created! SDL Error: %s\n",
				SDL_GetError());
		return -1;
	}

	//Initialize renderer color
	SDL_SetRenderDrawColor(gRenderer, 0xFF, 0xFF, 0xFF, 0xFF);

	//Initialize PNG loading
	if((IMG_Init(IMG_INIT_PNG) & IMG_INIT_PNG) == 0) {
		printf("SDL_image could not initialize! SDL_image Error: %s\n",
				IMG_GetError());
		return -1;
	}

	return 0;
}

short loadMedia()
{
	//Load splash texture
	if(LTexture_loadFromFile(&gSplashTexture, "splash.png")) {
		printf("Failed to load splash texture!\n");
		return -1;
	}

	return 0;
}

void close_all()
{
	//Free loaded images
	LTexture_free(&gSplashTexture);

	//Destroy window	
	SDL_DestroyRenderer(gRenderer);
	SDL_DestroyWindow(gWindow);
	gWindow = NULL;
	gRenderer = NULL;

	//Quit SDL subsystems
	IMG_Quit();
	SDL_Quit();
}

int worker(void* data)
{
	printf("%s starting...\n", data);

	//Pre thread random seeding
	srand(SDL_GetTicks());
	
	//Work 5 times
	int i;
	for(i = 0; i < 5; ++i) {
		SDL_Delay(16 + rand() % 32);
		SDL_AtomicLock(&gDataLock);		//Lock
		printf("%s gets %d\n", data, gData);	//Print pre work data
		gData = rand() % 256;			//"Work"
		printf("%s sets %d\n\n", data, gData);	//Print post work data
		SDL_AtomicUnlock(&gDataLock);		//Unlock
		SDL_Delay(16 + rand() % 640);		//Wait randomly
	}

	printf("%s finished!\n\n", data);

	return 0;
}


int main(int argc, char* args[])
{
	//Start up SDL and create window
	if(init()) {
		printf("Failed to initialize!\n");
		return -1;
	}

	//Load media
	if(loadMedia()) {
		printf("Failed to load media!\n");
		return -1;
	}

	//Event handler
	SDL_Event e;

	//Run the threads
	srand(SDL_GetTicks());
	SDL_Thread* threadA = SDL_CreateThread(
					worker, "Thread A", (void*)"Thread A");
	SDL_Delay(16 + rand() % 32);
	SDL_Thread* threadB = SDL_CreateThread(
					worker, "Thread B", (void*)"Thread B");
	
	//While application is running
	while(1)
	{
		while(SDL_PollEvent(&e) != 0)
			if(e.type == SDL_QUIT)
				goto equit;
		//Clear screen
		SDL_SetRenderDrawColor(gRenderer, 0xFF, 0xFF, 0xFF, 0xFF);
		SDL_RenderClear(gRenderer);

		//Render splash
		LTexture_render(&gSplashTexture, 0, 0, NULL);

		//Update screen
		SDL_RenderPresent(gRenderer);
	}

	//Wait for threads to finish
	SDL_WaitThread(threadA, NULL);
	SDL_WaitThread(threadB, NULL);
equit:
	//Free resources and close SDL
	close_all();

	return 0;
}
