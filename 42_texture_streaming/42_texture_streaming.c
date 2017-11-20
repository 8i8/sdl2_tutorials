/*This source code copyrighted by Lazy Foo' Productions (2004-2015)
and may not be redistributed without written permission.*/

//Using SDL, SDL_image, standard IO, and strings
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <stdio.h>

#define IMG_NUM		4
#define FRAME_DELAY	4

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

typedef struct {
	SDL_Surface* mImages[IMG_NUM];
	int mCurrentImage;
	int mDelayFrames;
} DataStream;

SDL_Window* gWindow = NULL;		//The window we'll be rendering to
SDL_Renderer* gRenderer = NULL;		//The window renderer
LTexture gStreamingTexture;		//Scene textures
DataStream gDataStream;			//Animation stream

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
	LTexture_free(lt);

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
	mTexture = SDL_CreateTextureFromSurface(gRenderer, textSurface);
	if(mTexture == NULL) {
		printf("Unable to create texture from rendered text! SDL Error: %s\n",
				SDL_GetError());
		return -1;
	}

	//Get image dimensions
	lt->mWidth = textSurface->w;
	lt->mHeight = textSurface->h;

	//Get rid of old surface
	SDL_FreeSurface(textSurface);

	//Return success
	return 0;
}
#endif
		
short LTexture_createBlank(LTexture *lt, int width, int height)
{
	//Create uninitialized texture
	lt->mTexture = SDL_CreateTexture(
					gRenderer,
					SDL_PIXELFORMAT_RGBA8888,
					SDL_TEXTUREACCESS_STREAMING,
					width,
					height);
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
			SDL_Rect* clip,
			double angle,
			SDL_Point* center,
			SDL_RendererFlip flip)
{
	//Set rendering space and render to screen
	SDL_Rect renderQuad = { x, y, lt->mWidth, lt->mHeight };

	//Set clip rendering dimensions
	if(clip != NULL) {
		renderQuad.w = clip->w;
		renderQuad.h = clip->h;
	}

	//Render to screen
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
	//Texture is locked
	if(lt->mPixels != NULL)
		memcpy(lt->mPixels, pixels, lt->mPitch * lt->mHeight);
}

int LTexture_getPitch(LTexture *lt)
{
	return lt->mPitch;
}

Uint32 LTexture_getPixel32(LTexture *lt, unsigned int x, unsigned int y)
{
	//Convert the pixels to 32 bit
	Uint32 *pixels = (Uint32*)lt->mPixels;

	//Get the pixel requested
	return pixels[(y * (lt->mPitch / 4)) + x];
}

void DataStream_init(DataStream *ds)
{
	int i;
	for (i = 0; i < IMG_NUM; i++)
		ds->mImages[IMG_NUM] = NULL;

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
			printf("Unable to load %s! SDL_image error: %s\n",
					path, IMG_GetError());
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

short init()
{
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
	//Load blank texture
	if(LTexture_createBlank(&gStreamingTexture, 64, 205)) {
		printf("Failed to create streaming texture!\n");
		return -1;
	}

	DataStream_init(&gDataStream);

	//Load data stream
	if(DataStream_loadMedia(&gDataStream)) {
		printf("Unable to load data stream!\n");
		return -1;
	}

	return 0;
}

void close_all()
{
	//Free loaded images
	LTexture_free(&gStreamingTexture);
	DataStream_free(&gDataStream);

	//Destroy window	
	SDL_DestroyRenderer(gRenderer);
	SDL_DestroyWindow(gWindow);
	gWindow = NULL;
	gRenderer = NULL;

	//Quit SDL subsystems
	IMG_Quit();
	SDL_Quit();
}

int main(int argc, char* args[])
{
	//Start up SDL and create window
	if(init()) {
		printf("Failed to initialize!\n");
		goto equit;
	}

	//Load media
	if(loadMedia()) {
		printf("Failed to load media!\n");
		goto equit;
	}

	SDL_Event e;

	//While application is running
	while(1)
	{
		while(SDL_PollEvent(&e) != 0)
			if(e.type == SDL_QUIT)
				goto equit;

		//Clear screen
		SDL_SetRenderDrawColor(gRenderer, 0xFF, 0xFF, 0xFF, 0xFF);
		SDL_RenderClear(gRenderer);

		//Copy frame from buffer
		LTexture_lockTexture(&gStreamingTexture);
		LTexture_copyPixels(
				&gStreamingTexture,
				DataStream_getBuffer(&gDataStream));
		LTexture_unlockTexture(&gStreamingTexture);

		//Render frame
		LTexture_render(
			&gStreamingTexture,
			(SCREEN_WIDTH - LTexture_getWidth(&gStreamingTexture)) / 2,
			(SCREEN_HEIGHT - LTexture_getHeight(&gStreamingTexture)) / 2,
			NULL, 0.0, NULL, 0);

		//Update screen
		SDL_RenderPresent(gRenderer);
	}
equit:
	//Free resources and close SDL
	close_all();

	return 0;
}
