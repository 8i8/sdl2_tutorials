/*
 * This programs demonstrates the rotation and the flippiing of a texture using
 * SDL_RendererFlip(). This program allso make a good example of
 * SDL_HINT_RENDER_SCALE_QUALITY.
 *
 * https://wiki.libsdl.org/SDL_RendererFlip
 */
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <stdio.h>
#include <math.h>

const int SCREEN_WIDTH = 640;
const int SCREEN_HEIGHT = 480;

typedef struct {
	SDL_Texture *mTexture;
	int mWidth;
	int mHeight;
} LTexture;

/* The flip type defined here is used to effect the rendered image */
typedef struct {
	SDL_RendererFlip flipType;
	double degrees;
} Data;

SDL_Window* gWindow = NULL;
SDL_Renderer* gRenderer = NULL;
LTexture gArrowTexture;

short init()
{
	if(SDL_Init(SDL_INIT_VIDEO) < 0) {
		SDL_Log("%s(), SDL_Init failed.", __func__);
		return -1;
	}

	if(SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "2") == 0)
		SDL_Log("%s() Linear texture filtering not enabled.",
				__func__);

	gWindow = SDL_CreateWindow(
					"SDL Tutorial",
					SDL_WINDOWPOS_UNDEFINED,
					SDL_WINDOWPOS_UNDEFINED,
					SCREEN_WIDTH,
					SCREEN_HEIGHT,
					SDL_WINDOW_SHOWN );
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

	if ((IMG_Init(IMG_INIT_PNG) & IMG_INIT_PNG) == 0) {
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

short loadFromFile(LTexture *lt, char *path)
{
	free_texture(lt);

	SDL_Texture* newTexture = NULL;

	SDL_Surface* loadedSurface = IMG_Load(path);
	if(loadedSurface == NULL) {
		SDL_SetError("%s(), IMG_Load failed to load \"%s\".",
				__func__, path);
		return -1;
	}

	SDL_SetColorKey(loadedSurface, SDL_TRUE, SDL_MapRGB(
				loadedSurface->format, 0, 0xFF, 0xFF));

	newTexture = SDL_CreateTextureFromSurface(gRenderer, loadedSurface);
	if(newTexture == NULL) {
		SDL_SetError("%s(), SDL_CreateTextureFromSurface failed.",
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
	SDL_Rect renderQuad = {x, y, lt->mWidth, lt->mHeight};

	if(clip != NULL) {
		renderQuad.w = clip->w;
		renderQuad.h = clip->h;
	}

	return SDL_RenderCopyEx(gRenderer, lt->mTexture, clip, &renderQuad,
				angle, center, flip);
}

int LTexture_getWidth(LTexture *lt)
{
	return lt->mWidth;
}

int LTexture_getHeight(LTexture *lt)
{
	return lt->mHeight;
}

short loadMedia()
{
	if(loadFromFile(&gArrowTexture, "arrow.png")) {
		SDL_Log("%s(), %s", __func__, SDL_GetError());
		return -1;
	}
	return 0;
}

void close_all()
{
	free_texture(&gArrowTexture);

	SDL_DestroyRenderer(gRenderer);
	SDL_DestroyWindow(gWindow);
	gWindow = NULL;
	gRenderer = NULL;
	
	IMG_Quit();
	SDL_Quit();
}

/* The values held in SDL_RendererFlip are set here with a key press. */
void get_key_press(SDL_Event *e, Data *d)
{
	switch(e->key.keysym.sym)
	{
		case SDLK_a:
		d->degrees -= 15;
		break;
		
		case SDLK_d:
		d->degrees += 15;
		break;

		case SDLK_q:
		d->flipType = SDL_FLIP_HORIZONTAL;
		break;

		case SDLK_w:
		d->flipType = SDL_FLIP_NONE;
		break;

		case SDLK_e:
		d->flipType = SDL_FLIP_VERTICAL;
		break;
	}
}

int main(int argc, char* argv[])
{
	SDL_Event e;
	Data d;
	d.degrees = 0;
	d.flipType = SDL_FLIP_NONE;

	if(init())
		goto equit;

	if(loadMedia())
		goto equit;

	while(1)
	{
		while(SDL_PollEvent( &e ) != 0)
		{
			if(e.type == SDL_QUIT)
				goto equit;

			if(e.type == SDL_KEYDOWN)
				get_key_press(&e, &d);
		}

		SDL_SetRenderDrawColor(gRenderer, 0xFF, 0xFF, 0xFF, 0xFF);
		SDL_RenderClear(gRenderer);

		LTexture_render(
			&gArrowTexture,
			(SCREEN_WIDTH - LTexture_getWidth(&gArrowTexture)) / 2,
			(SCREEN_HEIGHT - LTexture_getHeight(&gArrowTexture)) / 2,
			NULL,
			d.degrees,
			NULL,
			d.flipType);

		SDL_RenderPresent(gRenderer);
		SDL_Delay(16);
	}
equit:
	close_all();

	return 0;
}
