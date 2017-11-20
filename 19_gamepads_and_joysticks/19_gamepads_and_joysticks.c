/*This source code copyrighted by Lazy Foo' Productions (2004-2015)
and may not be redistributed without written permission.*/

//Using SDL, SDL_image, standard IO, math, and strings
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <stdio.h>
#include <math.h>

//Screen dimension constants
const int SCREEN_WIDTH = 640;
const int SCREEN_HEIGHT = 480;

//Analog joystick dead zone
const int JOYSTICK_DEAD_ZONE = 8000;

typedef struct {
	SDL_Texture *mTexture;
	int mWidth;
	int mHeight;
} LTexture;

SDL_Window* gWindow = NULL;		//The window we'll be rendering to
SDL_Renderer* gRenderer = NULL;		//The window renderer
LTexture gArrowTexture;			//Scene textures
SDL_Joystick* gGameController = NULL;	//Game Controller 1 handler

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
		printf( "Unable to create texture from %s! SDL Error: %s\n",
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
				TTF_GetError() );
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
	mWidth = textSurface->w;
	mHeight = textSurface->h;

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
	if( clip != NULL ) {
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

short init()
{
	//Initialize SDL
	if(SDL_Init(SDL_INIT_VIDEO | SDL_INIT_JOYSTICK) < 0)
	{
		printf("SDL could not initialize! SDL Error: %s\n",
				SDL_GetError());
		return -1;
	}

	//Set texture filtering to linear
	if(SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "1"))
		printf("Warning: Linear texture filtering not enabled!");

	//Check for joysticks
	if(SDL_NumJoysticks() < 1) {
		printf("Warning: No joysticks connected!\n");
		return -1;
	}

	//Load joystick
	gGameController = SDL_JoystickOpen(0);
	if(gGameController == NULL) {
		printf( "Warning: Unable to open game controller! SDL Error: %s\n",
				SDL_GetError());
		return -1;
	}

	//Create window
	gWindow = SDL_CreateWindow(
					"SDL Tutorial",
					SDL_WINDOWPOS_UNDEFINED,
					SDL_WINDOWPOS_UNDEFINED,
					SCREEN_WIDTH,
					SCREEN_HEIGHT,
					SDL_WINDOW_SHOWN);
	if(gWindow == NULL) {
		printf( "Window could not be created! SDL Error: %s\n",
				SDL_GetError() );
		return -1;
	}

	//Create vsynced renderer for window
	gRenderer = SDL_CreateRenderer(
					gWindow,
					-1,
					SDL_RENDERER_ACCELERATED
					| SDL_RENDERER_PRESENTVSYNC);
	if(gRenderer == NULL) {
		printf( "Renderer could not be created! SDL Error: %s\n",
				SDL_GetError() );
		return -1;
	}

	//Initialize renderer color
	SDL_SetRenderDrawColor( gRenderer, 0xFF, 0xFF, 0xFF, 0xFF );

	//Initialize PNG loading
	if((IMG_Init(IMG_INIT_PNG) & IMG_INIT_PNG) == 0) {
		printf( "SDL_image could not initialize! SDL_image Error: %s\n",
				IMG_GetError() );
		return -1;
	}

	return 0;
}

short loadMedia()
{
	//Load arrow texture
	if(LTexture_loadFromFile(&gArrowTexture, "arrow.png")) {
		printf( "Failed to load arrow texture!\n" );
		return -1;
	}
	return 0;
}

void close_all()
{
	//Free loaded images
	free_texture(&gArrowTexture);

	//Close game controller
	SDL_JoystickClose(gGameController);
	gGameController = NULL;

	//Destroy window	
	SDL_DestroyRenderer(gRenderer);
	SDL_DestroyWindow(gWindow);
	gWindow = NULL;
	gRenderer = NULL;

	//Quit SDL subsystems
	IMG_Quit();
	SDL_Quit();
}

void joystick_input(SDL_Event *e, int *xDir, int *yDir)
{
	//Motion on controller 0
	if(e->jaxis.which == 0) {
		if(e->jaxis.axis == 0)	//X axis motion
		{
			if(e->jaxis.value < -JOYSTICK_DEAD_ZONE)
				*xDir = -1;
			else if(e->jaxis.value > JOYSTICK_DEAD_ZONE)
				*xDir =  1;
			else
				*xDir = 0;
		}
		else if(e->jaxis.axis == 1)	//Y axis motion
		{
			if(e->jaxis.value < -JOYSTICK_DEAD_ZONE)
				*yDir = -1;
			else if(e->jaxis.value > JOYSTICK_DEAD_ZONE)
				*yDir =  1;
			else
				*yDir = 0;
		}
	}
}

int main(int argc, char* argv[])
{
	SDL_Event e;			//Event handler
	

	//Normalized direction
	int xDir = 0;
	int yDir = 0;

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

			
	//While application is running
	while(1)
	{
		//Handle events on queue
		while(SDL_PollEvent(&e) != 0)
		{
			//User requests quit
			if(e.type == SDL_QUIT)
				goto equit;

			if(e.type == SDL_JOYAXISMOTION)
				joystick_input(&e, &xDir, &yDir);
		}

		//Clear screen
		SDL_SetRenderDrawColor(gRenderer, 0xFF, 0xFF, 0xFF, 0xFF);
		SDL_RenderClear(gRenderer);

		//Calculate angle
		double joystickAngle;
		joystickAngle = atan2((double)yDir,(double)xDir) * (180.0 / M_PI);
		
		//Correct angle
		if(xDir == 0 && yDir == 0)
			joystickAngle = 0;

		//Render joystick 8 way angle
		LTexture_render(
				&gArrowTexture,
				(SCREEN_WIDTH - LTexture_getWidth(&gArrowTexture)) / 2,
				(SCREEN_HEIGHT - LTexture_getHeight(&gArrowTexture)) / 2,
				NULL,
				joystickAngle,
				NULL,
				SDL_FLIP_NONE);

		//Update screen
		SDL_RenderPresent(gRenderer);

		SDL_Delay(16);
	}
equit:
	//Free resources and close SDL
	close_all();

	return 0;
}
