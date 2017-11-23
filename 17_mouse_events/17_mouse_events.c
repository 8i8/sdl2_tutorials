/*
 * This program demonstrates the use of mouse events as an SDL_Event using
 * SDL_GetMouseState and various SDL_Event flags.
 *
 * https://wiki.libsdl.org/SDL_Event
 * https://wiki.libsdl.org/SDL_MouseButtonEvent
 *
 * SDL_MOUSEMOTION
 * SDL_MOUSEBUTTONDOWN
 * SDL_MOUSEBUTTONUP
 */
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <stdio.h>

const int SCREEN_WIDTH = 640;
const int SCREEN_HEIGHT = 480;

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

SDL_Window* gWindow = NULL;
SDL_Renderer* gRenderer = NULL;
SDL_Rect gSpriteClips[BUTTON_SPRITE_TOTAL];

LTexture gButtonSpriteSheetTexture;
LButton gButtons[TOTAL_BUTTONS];

short init()
{
	if(SDL_Init(SDL_INIT_VIDEO) < 0) {
		SDL_Log("%s(), SDL_Init failed.", __func__);
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
		SDL_Log("%s(), IMG_Init failed.", __func__);
		return -1;
	}

	return 0;
}

LTexture *free_texture(LTexture *lt)
{
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
	free_texture(lt);

	SDL_Texture* newTexture = NULL;

	SDL_Surface* loadedSurface = IMG_Load(path);
	if(loadedSurface == NULL) {
		SDL_Log("%s(), IMG_Load failed to load \"%s\".",
				__func__, path);
		return -1;
	}

	SDL_SetColorKey(
			loadedSurface,
			SDL_TRUE,
			SDL_MapRGB(loadedSurface->format, 0, 0xFF, 0xFF));

	newTexture = SDL_CreateTextureFromSurface(gRenderer, loadedSurface);
	if(newTexture == NULL) {
		SDL_Log("%s(), SDL_CreateTextureFromSurface failed.",
				__func__);
		return -1;
	}

	lt->mWidth = loadedSurface->w;
	lt->mHeight = loadedSurface->h;

	SDL_FreeSurface(loadedSurface);

	lt->mTexture = newTexture;

	return 0;
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
	SDL_Rect renderQuad = { x, y, lt->mWidth, lt->mHeight };

	if( clip != NULL ) {
		renderQuad.w = clip->w;
		renderQuad.h = clip->h;
	}

	return SDL_RenderCopyEx(gRenderer, lt->mTexture, clip,
				&renderQuad, angle, center, flip);
}

void LButton_setPosition(LButton *bt, int x, int y)
{
	bt->mPosition.x = x;
	bt->mPosition.y = y;
}

/*
 * The mouse events are listend for in main program loop.
 */
short LButton_handleEvent(LButton *lb, SDL_Event* e)
{
	if(
			e->type == SDL_MOUSEMOTION
			|| e->type == SDL_MOUSEBUTTONDOWN
			|| e->type == SDL_MOUSEBUTTONUP)
	{
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

		if(!active) {
			lb->mCurrentSprite = BUTTON_SPRITE_MOUSE_OUT;
			return 0;
		}

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
	LTexture_render(
			&gButtonSpriteSheetTexture,
			lb->mPosition.x,
			lb->mPosition.y,
			&gSpriteClips[lb->mCurrentSprite],
			0.0,
			NULL,
			SDL_FLIP_NONE);
}

short loadMedia()
{
	int i;
	if(LTexture_loadFromFile(&gButtonSpriteSheetTexture, "button.png") < 0)
		return -1;
	
	for(i = 0; i < BUTTON_SPRITE_TOTAL; ++i) {
		gSpriteClips[i].x = 0;
		gSpriteClips[i].y = i * 200;
		gSpriteClips[i].w = BUTTON_WIDTH;
		gSpriteClips[i].h = BUTTON_HEIGHT;
	}

	LButton_setPosition(&gButtons[0],0, 0);
	LButton_setPosition(&gButtons[1], SCREEN_WIDTH - BUTTON_WIDTH, 0);
	LButton_setPosition(&gButtons[2], 0, SCREEN_HEIGHT - BUTTON_HEIGHT);
	LButton_setPosition(&gButtons[3], SCREEN_WIDTH - BUTTON_WIDTH,
				   		SCREEN_HEIGHT - BUTTON_HEIGHT);
	return 0;
}

void close_all()
{
	free_texture(&gButtonSpriteSheetTexture);

	SDL_DestroyRenderer(gRenderer);
	SDL_DestroyWindow(gWindow);
	gWindow = NULL;
	gRenderer = NULL;

	IMG_Quit();
	SDL_Quit();
}

int main(int argc, char* argv[])
{
	int i;
	SDL_Event e;

	if(init())
		goto equit;

	if(loadMedia())
		goto equit;

	while(1)
	{
		while(SDL_PollEvent(&e)) {
			if(e.type == SDL_QUIT)
				goto equit;
			
			for(i = 0; i < TOTAL_BUTTONS; ++i)
				LButton_handleEvent(&gButtons[i], &e);
		}

		SDL_SetRenderDrawColor(gRenderer, 0xFF, 0xFF, 0xFF, 0xFF);
		SDL_RenderClear(gRenderer);

		for(i = 0; i < TOTAL_BUTTONS; ++i)
			LButton_render(&gButtons[i]);

		SDL_RenderPresent(gRenderer);
	}

equit:
	close_all();

	return 0;
}
