/*This source code copyrighted by Lazy Foo' Productions (2004-2015)
and may not be redistributed without written permission.*/

//Using SDL, SDL_image, standard IO, and strings
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <stdio.h>

//Screen dimension constants
const int SCREEN_WIDTH = 640;
const int SCREEN_HEIGHT = 480;

//Button constants
const int BUTTON_WIDTH = 300;
const int BUTTON_HEIGHT = 200;
#define TOTAL_BUTTONS	4

typedef enum {
	BUTTON_SPRITE_MOUSE_OUT = 0,
	BUTTON_SPRITE_MOUSE_OVER_MOTION = 1,
	BUTTON_SPRITE_MOUSE_DOWN = 2,
	BUTTON_SPRITE_MOUSE_UP = 3,
	BUTTON_SPRITE_TOTAL = 4
} LButtonSprite;

typedef struct {
	SDL_Texture *mTexture;
	int mWidth;
	int mHeight;
} LTexture;

typedef struct {
	SDL_Point mPosition;
	LButtonSprite mCurrentSprite;
} LButton;

SDL_Window* gWindow = NULL;			//The window we'll be rendering to
SDL_Renderer* gRenderer = NULL;			//The window renderer
SDL_Rect gSpriteClips[BUTTON_SPRITE_TOTAL];	//Mouse button sprites

LTexture gButtonSpriteSheetTexture;
LButton gButtons[TOTAL_BUTTONS]; 		//Buttons objects

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
			gFont, textureText.c_str(), textColor);
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

void LButton_init(LButton *lb)
{
	lb->mPosition.x = 0;
	lb->mPosition.y = 0;

	lb->mCurrentSprite = BUTTON_SPRITE_MOUSE_OUT;
}

void LButton_setPosition(LButton *bt, int x, int y)
{
	bt->mPosition.x = x;
	bt->mPosition.y = y;
}

short LButton_handleEvent(LButton *lb, SDL_Event* e)
{
	//If mouse event happened
	if(		   e->type == SDL_MOUSEMOTION
			|| e->type == SDL_MOUSEBUTTONDOWN
			|| e->type == SDL_MOUSEBUTTONUP)
	{
		//Get mouse position
		int x, y;
		short active = 1;

		SDL_GetMouseState(&x, &y);

		if(x < lb->mPosition.x) {
			active = 0;
		}
		else if(x > lb->mPosition.x + BUTTON_WIDTH) {
			active = 0;
		}
		else if(y < lb->mPosition.y) {
			active = 0;
		}
		else if(y > lb->mPosition.y + BUTTON_HEIGHT) {
			active = 0;
		}

		//Mouse is outside button
		if(!active) {
			lb->mCurrentSprite = BUTTON_SPRITE_MOUSE_OUT;
			return 0;
		}

		//Set mouse over sprite
		switch(e->type)
		{
			case SDL_MOUSEMOTION:
			lb->mCurrentSprite = BUTTON_SPRITE_MOUSE_OVER_MOTION;
			break;
		
			case SDL_MOUSEBUTTONDOWN:
			lb->mCurrentSprite = BUTTON_SPRITE_MOUSE_DOWN;
			break;
			
			case SDL_MOUSEBUTTONUP:
			lb->mCurrentSprite = BUTTON_SPRITE_MOUSE_UP;
			break;
		}
	}
	return 0;
}
	
void LButton_render(LButton *lb)
{
	//Show current button sprite
	LTexture_render(
			&gButtonSpriteSheetTexture,
			lb->mPosition.x,
			lb->mPosition.y,
			&gSpriteClips[lb->mCurrentSprite],
			0.0,
			NULL,
			SDL_FLIP_NONE);
}

short init()
{
	//Initialize SDL
	if(SDL_Init(SDL_INIT_VIDEO) < 0)
	{
		printf("SDL could not initialize! SDL Error: %s\n",
				SDL_GetError());
		return -1;
	}

	//Set texture filtering to linear
	if(SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "1") == 0) {
		printf("Warning: Linear texture filtering not enabled!");
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

	return 0;
}

short loadMedia()
{
	int i;
	//Load sprites
	if(LTexture_loadFromFile(&gButtonSpriteSheetTexture, "button.png") < 0) {
		printf( "Failed to load button sprite texture!\n" );
		return -1;
	}
	
	//Set sprites
	for(i = 0; i < BUTTON_SPRITE_TOTAL; ++i) {
		gSpriteClips[i].x = 0;
		gSpriteClips[i].y = i * 200;
		gSpriteClips[i].w = BUTTON_WIDTH;
		gSpriteClips[i].h = BUTTON_HEIGHT;
	}

	//Set buttons in corners
	LButton_setPosition(&gButtons[0],0, 0);
	LButton_setPosition(&gButtons[1], SCREEN_WIDTH - BUTTON_WIDTH, 0);
	LButton_setPosition(&gButtons[2], 0, SCREEN_HEIGHT - BUTTON_HEIGHT);
	LButton_setPosition(&gButtons[3], SCREEN_WIDTH - BUTTON_WIDTH,
				   		SCREEN_HEIGHT - BUTTON_HEIGHT);
	return 0;
}

void close_all()
{
	//Free loaded images
	free_texture(&gButtonSpriteSheetTexture);

	//Destroy window	
	SDL_DestroyRenderer(gRenderer);
	SDL_DestroyWindow(gWindow);
	gWindow = NULL;
	gRenderer = NULL;

	//Quit SDL subsystems
	IMG_Quit();
	SDL_Quit();
}

int main(int argc, char* argv[])
{
	int i;
	SDL_Event e;		//Event handler

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
		while(SDL_PollEvent(&e)) {
			if(e.type == SDL_QUIT)
				goto equit;
			
			//Handle button events
			for(i = 0; i < TOTAL_BUTTONS; ++i)
				LButton_handleEvent(&gButtons[i], &e);
		}

		//Clear screen
		SDL_SetRenderDrawColor(gRenderer, 0xFF, 0xFF, 0xFF, 0xFF);
		SDL_RenderClear(gRenderer);

		//Render buttons
		for(i = 0; i < TOTAL_BUTTONS; ++i)
			LButton_render(&gButtons[i]);

		//Update screen
		SDL_RenderPresent(gRenderer);
	}

	//Free resources and close SDL
equit:
	close_all();

	return 0;
}
