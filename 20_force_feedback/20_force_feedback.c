/*This source code copyrighted by Lazy Foo' Productions (2004-2015)
and may not be redistributed without written permission.*/

//Using SDL, SDL_image, SDL_ttf, standard IO, math, and strings
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <stdio.h>
#include <math.h>

//Screen dimension constants
const int SCREEN_WIDTH = 640;
const int SCREEN_HEIGHT = 480;

typedef struct {
	SDL_Texture *mTexture;
	int mWidth;
	int mHeight;
} LTexture;

SDL_Window* gWindow = NULL;		//The window we'll be rendering to
SDL_Renderer* gRenderer = NULL;		//The window renderer
LTexture gSplashTexture;		//Scene texture

//Game Controller 1 handler with force feedback
SDL_Joystick* gGameController = NULL;
SDL_Haptic* gControllerHaptic = NULL;

LTexture *free_texture(LTexture *lt)
{
	//Free texture if it exists
	if(lt->mTexture != NULL) {
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

short init()
{
	//Initialize SDL
	if(SDL_Init(SDL_INIT_VIDEO | SDL_INIT_JOYSTICK | SDL_INIT_HAPTIC)) {
		printf( "SDL could not initialize! SDL Error: %s\n",
				SDL_GetError());
		return -1;
	}

	//Set texture filtering to linear
	if(SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "1"))
		printf( "Warning: Linear texture filtering not enabled!" );

	//Check for joysticks
	if(SDL_NumJoysticks() < 1) {
		printf( "Warning: No joysticks connected!\n" );
		return -1;
	}

	//Load joystick
	gGameController = SDL_JoystickOpen(0);
	if(gGameController == NULL) {
		printf("Warning: Unable to open game controller! SDL Error: %s\n",
				SDL_GetError());
		return -1;
	}

	//Get controller haptic device
	gControllerHaptic = SDL_HapticOpenFromJoystick(gGameController);
	if(gControllerHaptic == NULL) {
		printf("Warning: Controller does not support haptics! SDL Error: %s\n",
				SDL_GetError());
		return -1;
	}

	//Get initialize rumble
	if(SDL_HapticRumbleInit(gControllerHaptic) < 0) {
		printf("Warning: Unable to initialize rumble! SDL Error: %s\n",
				SDL_GetError());
		return -1;
	}

	//Create window
	gWindow = SDL_CreateWindow(
					"SDL Tutorial",
					SDL_WINDOWPOS_UNDEFINED,
					SDL_WINDOWPOS_UNDEFINED,
					SCREEN_WIDTH, SCREEN_HEIGHT,
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
		printf( "Renderer could not be created! SDL Error: %s\n",
				SDL_GetError() );
		return -1;
	}

	//Initialize renderer color
	SDL_SetRenderDrawColor(gRenderer, 0xFF, 0xFF, 0xFF, 0xFF);

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
	//Load press texture
	if(LTexture_loadFromFile(&gSplashTexture, "splash.png")) {
		printf("Failed to load splash texture!\n");
		return -1;
	}
	return 0;
}

void close_all()
{
	//Free loaded images
	free_texture(&gSplashTexture);

	//Close game controller with haptics
	SDL_HapticClose(gControllerHaptic);
	SDL_JoystickClose(gGameController);
	gGameController = NULL;
	gControllerHaptic = NULL;

	//Destroy window	
	SDL_DestroyRenderer(gRenderer);
	SDL_DestroyWindow(gWindow);
	gWindow = NULL;
	gRenderer = NULL;

	//Quit SDL subsystems
	IMG_Quit();
	SDL_Quit();
}

void joystic_rumble()
{
	if(SDL_HapticRumblePlay(gControllerHaptic, 0.75, 500))
		printf("Warning: Unable to play rumble! %s\n", SDL_GetError());
}

int main(int argc, char* argv[])
{
	SDL_Event e;		//Event handler
			
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

	//While application is running
	while(1)
	{
		//Handle events on queue
		while(SDL_PollEvent(&e) != 0)
		{
			//User requests quit
			if( e.type == SDL_QUIT )
				goto equit;

			//Joystick button press
			if(e.type == SDL_JOYBUTTONDOWN)
				joystic_rumble();
		}

		//Clear screen
		SDL_SetRenderDrawColor(gRenderer, 0xFF, 0xFF, 0xFF, 0xFF);
		SDL_RenderClear(gRenderer);

		//Render current texture
		LTexture_render(&gSplashTexture, 0, 0, NULL, 0.0, NULL,
						SDL_FLIP_NONE);

		//Update screen
		SDL_RenderPresent(gRenderer);
	}
equit:
	//Free resources and close SDL
	close_all();

	return 0;
}
