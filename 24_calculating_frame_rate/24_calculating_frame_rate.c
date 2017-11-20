/*This source code copyrighted by Lazy Foo' Productions (2004-2015)
and may not be redistributed without written permission.*/

//Using SDL, SDL_image, SDL_ttf, standard IO, strings, and string streams
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_ttf.h>
#include <stdio.h>

//Screen dimension constants
const int SCREEN_WIDTH = 640;
const int SCREEN_HEIGHT = 480;

//Texture wrapper class
typedef struct {
	SDL_Texture *mTexture;
	int mWidth;
	int mHeight;
} LTexture;

typedef struct {
	Uint32 mStartTicks;
	Uint32 mPausedTicks;
	short mPaused;
	short mStarted;
} LTimer;

SDL_Window* gWindow = NULL;		//The window we'll be rendering to
SDL_Renderer* gRenderer = NULL;		//The window renderer
TTF_Font* gFont = NULL;			//Globally used font
LTexture gFPSTextTexture;		//Scene textures

LTexture *free_texture(LTexture *lt)
{
	//Free texture if it exists
	if(lt->mTexture != NULL)
	{
		SDL_DestroyTexture(lt->mTexture);
		lt->mTexture = NULL;
		lt->mWidth = 0;
		lt->mHeight = 0;
	}
	return lt;
}

short LTexture_loadFromFile(LTexture *lt, char *path)
{
	//Get rid of preexisting texture
	free_texture(lt);

	//The final texture
	SDL_Texture* newTexture = NULL;

	//Load image at specified path
	SDL_Surface* loadedSurface = IMG_Load(path);
	if(loadedSurface == NULL) {
		printf("Unable to load image %s! SDL_image Error: %s\n",
				path, IMG_GetError());
		return -1;
	}

	//Color key image
	SDL_SetColorKey(
			loadedSurface,
			SDL_TRUE,
			SDL_MapRGB(loadedSurface->format, 0, 0xFF, 0xFF));

	//Create texture from surface pixels
	newTexture = SDL_CreateTextureFromSurface(gRenderer, loadedSurface);
	if(newTexture == NULL) {
		printf("Unable to create texture from %s! SDL Error: %s\n",
				path, SDL_GetError());
		return 1;
	}

	//Get image dimensions
	lt->mWidth = loadedSurface->w;
	lt->mHeight = loadedSurface->h;

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
	free_texture(lt);

	//Render text surface
	SDL_Surface* textSurface = TTF_RenderText_Solid(
						gFont, textureText, textColor);
	if(textSurface == NULL) {
		printf("Unable to render text surface! SDL_ttf Error: %s\n",
				TTF_GetError());
		return -1;
	}

	//Create texture from surface pixels
	lt->mTexture = SDL_CreateTextureFromSurface(gRenderer, textSurface);
	if(lt->mTexture == NULL) {
		printf("Unable to create texture from rendered text! SDL Error: %s\n",
				SDL_GetError() );
		return -1;
	}

	//Get image dimensions
	lt->mWidth = textSurface->w;
	lt->mHeight = textSurface->h;

	//Get rid of old surface
	SDL_FreeSurface(textSurface);
	
	return 0;
}
#endif

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

short LTexture_render(
			LTexture *lt,
			int x,
			int y,
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
	return SDL_RenderCopyEx(gRenderer, lt->mTexture, clip,
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

void LTimer_init(LTimer *lt)
{
	//Initialize the variables
	lt->mStartTicks = 0;
	lt->mPausedTicks = 0;
	lt->mPaused = 0;
	lt->mStarted = 0;
}

void LTimer_start(LTimer *lt)
{
	lt->mStarted = 1;
	lt->mPaused = 0;
	lt->mStartTicks = SDL_GetTicks();
	lt->mPausedTicks = 0;
}

void LTimer_stop(LTimer *lt)
{
	//Stop the timer
	lt->mStarted = 0;

	//Unpause the timer
	lt->mPaused = 0;

	//Clear tick variables
	lt->mStartTicks = 0;
	lt->mPausedTicks = 0;
}

void LTimer_pause(LTimer *lt)
{
	//If the timer is running and isn't already paused
	if(lt->mStarted && !lt->mPaused) {
		lt->mPaused = 1;
		lt->mPausedTicks = SDL_GetTicks() - lt->mStartTicks;
		lt->mStartTicks = 0;
	}
}

void LTimer_unpause(LTimer *lt)
{
	if(lt->mStarted && lt->mPaused) {
		lt->mPaused = 0;
		lt->mStartTicks = SDL_GetTicks() - lt->mPausedTicks;
		lt->mPausedTicks = 0;
	}
}

Uint32 LTimer_getTicks(LTimer *lt)
{
	if(lt->mStarted) {
		if(lt->mPaused)
			return lt->mPausedTicks;
		else
			return SDL_GetTicks() - lt->mStartTicks;
	}
	return 0;
}

short LTimer_isStarted(LTimer *lt)
{
	return lt->mStarted;
}

short LTimer_isPaused(LTimer *lt)
{
	return lt->mPaused && lt->mStarted;
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

	//Create vsynced renderer for window
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
				IMG_GetError() );
		return -1;
	}

	 //Initialize SDL_ttf
	if(TTF_Init() < 0) {
		printf("SDL_ttf could not initialize! SDL_ttf Error: %s\n",
				TTF_GetError());
		return -1;
	}

	return 0;
}

short loadMedia()
{
	gFont = TTF_OpenFont("lazy.ttf", 28);
	if(gFont == NULL) {
		printf("Failed to load lazy font! SDL_ttf Error: %s\n",
				TTF_GetError());
		return -1;
	}
	return 0;
}

void close_all()
{
	//Free loaded images
	free_texture(&gFPSTextTexture);

	//Free global font
	TTF_CloseFont(gFont);
	gFont = NULL;

	//Destroy window	
	SDL_DestroyRenderer(gRenderer);
	SDL_DestroyWindow(gWindow);
	gWindow = NULL;
	gRenderer = NULL;

	//Quit SDL subsystems
	TTF_Quit();
	IMG_Quit();
	SDL_Quit();
}

int main(int argc, char* args[])
{
	char *text = "Average Frames Per Second ";
	int max_char_uint32 = 11;
	char timeText[strlen(text) + max_char_uint32];
	int countedFrames = 0;
	float avgFPS;

	//Start up SDL and create window
	if(init()) {
		printf( "Failed to initialize!\n" );
		goto equit;
	}

	//Load media
	if(loadMedia()) {
		printf( "Failed to load media!\n" );
		goto equit;
	}

	SDL_Event e;
	SDL_Color textColor = { 0, 0, 0, 255 };
	LTimer fpsTimer;

	LTimer_init(&fpsTimer);
	LTimer_start(&fpsTimer);

	//While application is running
	while(1)
	{
		while(SDL_PollEvent( &e ) != 0)
			if(e.type == SDL_QUIT)
				goto equit;

		//Calculate and correct fps
		avgFPS = countedFrames / (LTimer_getTicks(fpsTimer) / 1000.f);
		if(avgFPS > 2000000)
			avgFPS = 0;
		
		//Set text to be rendered
		sprintf(timeText, "%s %6.4f", text, avgFPS);

		//Render text
		if(LTexture_loadFromRenderedText(
					&gFPSTextTexture, timeText, textColor))
			printf("Unable to render FPS texture!\n");

		//Clear screen
		SDL_SetRenderDrawColor(gRenderer, 0xFF, 0xFF, 0xFF, 0xFF);
		SDL_RenderClear(gRenderer);

		//Render textures
		LTexture_render(
				&gFPSTextTexture,
				(SCREEN_WIDTH - LTexture_getWidth(
							&gFPSTextTexture)) / 2,
				(SCREEN_HEIGHT - LTexture_getHeight(
							&gFPSTextTexture)) / 2,
				NULL, 0.0, NULL, SDL_FLIP_NONE);

		//Update screen
		SDL_RenderPresent(gRenderer);
		++countedFrames;
	}
equit:
	//Free resources and close SDL
	close_all();

	return 0;
}

