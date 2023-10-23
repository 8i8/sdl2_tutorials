/*
 * Mouse Events
 *
 * Like with key presses, SDL has event structures to handle mouse events such
 * as mouse motion, mouse button presses, and mouse button releasing. In this
 * tutorial we'll make a bunch of buttons we can interact with.
 *
 * We're making a slight modification to the texture class. For this tutorial
 * we won't be using SDL_ttf to render text. This means we don't need the
 * loadFromRenderedText function.
 */
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>

#define SCREEN_WIDTH	640
#define SCREEN_HEIGHT	480
#define BUTTON_WIDTH	300
#define BUTTON_HEIGHT	200
#define TOTAL_BUTTONS	4

/*
 * For this tutorial we'll have 4 buttons on the screen. Depending on whether
 * the mouse moved over, clicked on, released on, or moved out of the button
 * we'll display a different sprite. These constants are here to define all
 * this.
 */
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

/*
 * Here is the struct to represent a button. It has related functions, a
 * position setter, an event handler for the event loop, and a rendering
 * function. It also has a position and a sprite enumeration so we know which
 * sprite to render for the button.
 */
typedef struct {
	SDL_Point mPosition;
	LButtonSprite mCurrentSprite;
} LButton;

SDL_Window* gWindow = NULL;
SDL_Renderer* gRenderer = NULL;
SDL_Rect gSpriteClips[BUTTON_SPRITE_TOTAL];

LTexture gButtonSpriteSheetTexture;
LButton gButtons[TOTAL_BUTTONS];

short init(void)
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
		SDL_Log("%s(), IMG_Load failed. %s", __func__, IMG_GetError());
		return -1;
	}

	SDL_SetColorKey(
			loadedSurface,
			SDL_TRUE,
			SDL_MapRGB(loadedSurface->format, 0, 0xFF, 0xFF));

	newTexture = SDL_CreateTextureFromSurface(gRenderer, loadedSurface);
	if(newTexture == NULL) {
		SDL_Log("%s(), SDL_CreateTextureFromSurface failed. %s", __func__, SDL_GetError());
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
/*
 * Here is the position setting function.
 */
void LButton_setPosition(LButton *bt, int x, int y)
{
	bt->mPosition.x = x;
	bt->mPosition.y = y;
}

/*
 * Here's the meat of the tutorial where we handle the mouse events. This
 * function will be called in the event loop and will handle an event taken
 * from the event queue for an individual button.
 *
 * First we check if the event coming in is a mouse event specifically a mouse
 * motion event (when the mouse moves), a mouse button down event (when you
 * click a mouse button), or a mouse button up event (when you release a mouse
 * click).
 *
 * If one of these mouse events do occur, we check the mouse position using
 * SDL_GetMouseState. Depending on whether the mouse is over the button or not,
 * we'll want to display different sprites.
 *
 * https://wiki.libsdl.org/SDL_GetMouseState
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
/*
 * Here we want to check if the mouse is inside the button or not. Since we use
 * a different coordinate system with SDL, the origin of the button is at the
 * top left. This means every x coordinate less than the x position is outside
 * of the button and every y coordinate less than the y position is too.
 * Everything right of the button is the x position + the width and everything
 * below the button is the y position + the height.
 *
 * This is what this piece of code does. If the mouse position is in any way
 * outside the button, it marks the inside marker as false. Otherwise it
 * remains the initial true value.
 */
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
/*
 * Finally, we set the button sprite depending on whether the mouse is inside
 * the button and the mouse event.
 *
 * If the mouse isn't inside the button, we set the mouse out sprite. If the
 * mouse is inside we set the sprite to mouse over on a mouse motion, mouse
 * down on a mouse button press, and mouse up on a mouse button release.
 */
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
	
/*
 * In the rendering function, we just render the current button sprite at the
 * button position. 
 */
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

short loadMedia(void)
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

void close_all(void)
{
	free_texture(&gButtonSpriteSheetTexture);

	SDL_DestroyRenderer(gRenderer);
	SDL_DestroyWindow(gWindow);
	gWindow = NULL;
	gRenderer = NULL;

	IMG_Quit();
	SDL_Quit();
}

/*
 * Here is our main loop. In the event loop, we handle the quit event and the
 * events for all the buttons. In the rendering section, all the buttons are
 * rendered to the screen.
 *
 * There are also mouse wheel events which weren't covered here, but if you
 * look at the documentation and play around with it it shouldn't be too hard
 * to figure out.
 */
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
