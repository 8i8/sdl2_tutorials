/*This source code copyrighted by Lazy Foo' Productions (2004-2015)
and may not be redistributed without written permission.*/

//Using SDL, SDL Threads, SDL_image, standard IO, and, strings
#include <SDL2/SDL.h>
#include <SDL2/SDL_thread.h>
#include <SDL2/SDL_image.h>
#include <stdio.h>
#include <string>

//Screen dimension constants
const int SCREEN_WIDTH = 640;
const int SCREEN_HEIGHT = 480;

//Texture wrapper class
class LTexture
{
	public:
		//Initializes variables
		LTexture();

		//Deallocates memory
		~LTexture();

		//Loads image at specified path
		bool loadFromFile(std::string path);
		
		#ifdef _SDL_TTF_H
		//Creates image from font string
		bool loadFromRenderedText(
				std::string textureText,
				SDL_Color textColor);
		#endif

		//Creates blank texture
		bool createBlank(
				int width, int height,
				SDL_TextureAccess = SDL_TEXTUREACCESS_STREAMING);

		//Deallocates texture
		void free();

		//Set color modulation
		void setColor(Uint8 red, Uint8 green, Uint8 blue);

		//Set blending
		void setBlendMode(SDL_BlendMode blending);

		//Set alpha modulation
		void setAlpha(Uint8 alpha);
		
		//Renders texture at given point
		void render(
				int x, int y,
				SDL_Rect* clip = NULL,
				double angle = 0.0,
				SDL_Point* center = NULL,
				SDL_RendererFlip flip = SDL_FLIP_NONE);

		//Set self as render target
		void setAsRenderTarget();

		//Gets image dimensions
		int getWidth();
		int getHeight();

		//Pixel manipulators
		bool lockTexture();
		bool unlockTexture();
		void* getPixels();
		void copyPixels(void* pixels);
		int getPitch();
		Uint32 getPixel32(unsigned int x, unsigned int y);

	private:
		//The actual hardware texture
		SDL_Texture* mTexture;
		void* mPixels;
		int mPitch;

		//Image dimensions
		int mWidth;
		int mHeight;
};

//Our test thread function
int threadFunction(void* data);

//The window we'll be rendering to
SDL_Window* gWindow = NULL;

//The window renderer
SDL_Renderer* gRenderer = NULL;

//Scene textures
LTexture gSplashTexture;

LTexture::LTexture()
{
	//Initialize
	mTexture = NULL;
	mWidth = 0;
	mHeight = 0;
	mPixels = NULL;
	mPitch = 0;
}

LTexture::~LTexture()
{
	//Deallocate
	free();
}

bool LTexture::loadFromFile(std::string path)
{
	//Get rid of preexisting texture
	free();

	//The final texture
	SDL_Texture* newTexture = NULL;

	//Load image at specified path
	SDL_Surface* loadedSurface = IMG_Load(path.c_str());
	if(loadedSurface == NULL) {
		printf("Unable to load image %s! SDL_image Error: %s\n",
				path.c_str(), IMG_GetError());
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
				&mPixels,
				&mPitch);

	//Copy loaded/formatted surface pixels
	memcpy(
				mPixels,
				formattedSurface->pixels,
				formattedSurface->pitch * formattedSurface->h);

	//Get image dimensions
	mWidth = formattedSurface->w;
	mHeight = formattedSurface->h;

	//Get pixel data in editable format
	Uint32* pixels = (Uint32*)mPixels;
	int pixelCount = (mPitch / 4) * mHeight;

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
	mPixels = NULL;

	//Get rid of old formatted surface
	SDL_FreeSurface(formattedSurface);
		
	//Get rid of old loaded surface
	SDL_FreeSurface(loadedSurface);

	//Return success
	mTexture = newTexture;
	return 0;
}

#ifdef _SDL_TTF_H
bool LTexture::loadFromRenderedText(std::string textureText, SDL_Color textColor)
{
	//Get rid of preexisting texture
	free();

	//Render text surface
	SDL_Surface* textSurface = TTF_RenderText_Solid(
							gFont,
							textureText.c_str(),
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
	mWidth = textSurface->w;
	mHeight = textSurface->h;

	//Get rid of old surface
	SDL_FreeSurface(textSurface);
	
	//Return success
	return mTexture != NULL;
}
#endif
		
bool LTexture::createBlank(int width, int height, SDL_TextureAccess access)
{
	//Create uninitialized texture
	mTexture = SDL_CreateTexture(
					gRenderer,
					SDL_PIXELFORMAT_RGBA8888,
					access, width, height);
	if(mTexture == NULL) {
		printf("Unable to create blank texture! SDL Error: %s\n",
				SDL_GetError());
		return -1;
	}

	mWidth = width;
	mHeight = height;

	return 0;
}

void LTexture::free()
{
	//Free texture if it exists
	if(mTexture != NULL) {
		SDL_DestroyTexture(mTexture);
		mTexture = NULL;
		mWidth = 0;
		mHeight = 0;
		mPixels = NULL;
		mPitch = 0;
	}
}

void LTexture::setColor(Uint8 red, Uint8 green, Uint8 blue)
{
	//Modulate texture rgb
	SDL_SetTextureColorMod(mTexture, red, green, blue);
}

void LTexture::setBlendMode(SDL_BlendMode blending)
{
	//Set blending function
	SDL_SetTextureBlendMode(mTexture, blending);
}
		
void LTexture::setAlpha(Uint8 alpha)
{
	//Modulate texture alpha
	SDL_SetTextureAlphaMod(mTexture, alpha);
}

void LTexture::render(
			int x, int y,
			SDL_Rect* clip,
			double angle,
			SDL_Point* center,
			SDL_RendererFlip flip)
{
	//Set rendering space and render to screen
	SDL_Rect renderQuad = { x, y, mWidth, mHeight };

	//Set clip rendering dimensions
	if(clip != NULL)
	{
		renderQuad.w = clip->w;
		renderQuad.h = clip->h;
	}

	//Render to screen
	SDL_RenderCopyEx(gRenderer, mTexture, clip,
				&renderQuad, angle, center, flip);
}

void LTexture::setAsRenderTarget()
{
	//Make self render target
	SDL_SetRenderTarget(gRenderer, mTexture);
}

int LTexture::getWidth()
{
	return mWidth;
}

int LTexture::getHeight()
{
	return mHeight;
}

bool LTexture::lockTexture()
{
	//Texture is already locked
	if(mPixels != NULL) {
		printf("Texture is already locked!\n");
		return -1;
	}

	if(SDL_LockTexture(mTexture, NULL, &mPixels, &mPitch) != 0) {
		printf("Unable to lock texture! %s\n", SDL_GetError());
		return -1;
	}

	return 0;
}

bool LTexture::unlockTexture()
{
	//Texture is not locked
	if(mPixels == NULL) {
		printf("Texture is not locked!\n");
		return -1;
	}

	SDL_UnlockTexture(mTexture);
	mPixels = NULL;
	mPitch = 0;

	return 0;
}

void* LTexture::getPixels()
{
	return mPixels;
}

void LTexture::copyPixels(void* pixels)
{
	//Texture is locked
	if(mPixels != NULL)
		memcpy(mPixels, pixels, mPitch * mHeight);
}

int LTexture::getPitch()
{
	return mPitch;
}

Uint32 LTexture::getPixel32(unsigned int x, unsigned int y)
{
	//Convert the pixels to 32 bit
	Uint32 *pixels = (Uint32*)mPixels;

	//Get the pixel requested
	return pixels[(y * (mPitch / 4)) + x];
}

bool init()
{
	//Initialize SDL
	if(SDL_Init(SDL_INIT_VIDEO) < 0) {
		printf("SDL could not initialize! SDL Error: %s\n",
				SDL_GetError());
		return -1;
	}

	//Set texture filtering to linear
	if(!SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "1"))
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

bool loadMedia()
{
	//Load splash texture
	if(gSplashTexture.loadFromFile("splash.png")) {
		printf("Failed to load splash texture!\n");
		return -1;
	}

	return 0;
}

void close_all()
{
	//Free loaded images
	gSplashTexture.free();

	//Destroy window	
	SDL_DestroyRenderer(gRenderer);
	SDL_DestroyWindow(gWindow);
	gWindow = NULL;
	gRenderer = NULL;

	//Quit SDL subsystems
	IMG_Quit();
	SDL_Quit();
}

int threadFunction(void* data)
{
	//Print incoming data
	printf("Running thread with value = %d\n", (int)data);

	return 0;
}

int main(int argc, char* args[])
{
	//Start up SDL and create window
	if(init()) {
		printf("Failed to initialize!\n");
		return 1;
	}

	//Load media
	if(loadMedia()) {
		printf("Failed to load media!\n");
		return 1;
	}

	int data = 101;

	//Run the thread
	SDL_Thread* threadID = SDL_CreateThread(
						threadFunction,
						"LazyThread",
						(void*)data);
	//Event handler
	SDL_Event e;

	while(1)
	{
		while(SDL_PollEvent(&e) != 0)
			if(e.type == SDL_QUIT)
				goto equit;
		//Clear screen
		SDL_SetRenderDrawColor(gRenderer, 0xFF, 0xFF, 0xFF, 0xFF);
		SDL_RenderClear(gRenderer);

		//Render prompt
		gSplashTexture.render(0, 0);

		//Update screen
		SDL_RenderPresent(gRenderer);
	}

	//Remove timer in case the call back was not called
	SDL_WaitThread(threadID, NULL);
equit:
	//Free resources and close SDL
	close_all();

	return 0;
}

