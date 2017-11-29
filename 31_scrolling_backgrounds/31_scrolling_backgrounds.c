/*
 * Scrolling Backgrounds
 *
 * Often times in games you may want a infinite or looping background. With
 * scrolling backgrounds, you can cycle a background that goes on forever.
 *
 * Say if we want to move around a dot an a infinite background. All we have to
 * do is render two iterations of the background next to each other and move
 * them a little every frame. When the background have moved completely over,
 * you can reset the motion:
 *
 * For this tutorial we'll be using a plain version of the dot that just stays
 * on screen.
 */
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <stdio.h>

#define  SCREEN_WIDTH	640
#define  SCREEN_HEIGHT	480
#define  DOT_WIDTH	20
#define  DOT_HEIGHT	20
#define  DOT_VEL	5

typedef struct {
	SDL_Texture *mTexture;
	int mWidth;
	int mHeight;
} LTexture;

typedef struct {
	int mPosX, mPosY;
	int mVelX, mVelY;
} Dot;

SDL_Window* gWindow = NULL;
SDL_Renderer* gRenderer = NULL;
LTexture gDotTexture;
LTexture gBGTexture;

short init()
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

short LTexture_render(LTexture *lt, int x, int y, SDL_Rect* clip)
{
	SDL_Rect renderQuad = {x, y, lt->mWidth, lt->mHeight};

	if(clip != NULL) {
		renderQuad.w = clip->w;
		renderQuad.h = clip->h;
	}

	return SDL_RenderCopy(gRenderer, lt->mTexture, clip, &renderQuad);
}

void Dot_init(Dot *d)
{
	d->mPosX = 0;
	d->mPosY = 0;

	d->mVelX = 0;
	d->mVelY = 0;
}

void Dot_handleEvent(Dot *d, SDL_Event *e)
{
	if(e->type == SDL_KEYDOWN && e->key.repeat == 0)
	{
		switch(e->key.keysym.sym)
		{
			case SDLK_UP: 	d->mVelY -= DOT_VEL; break;
			case SDLK_DOWN: d->mVelY += DOT_VEL; break;
			case SDLK_LEFT: d->mVelX -= DOT_VEL; break;
			case SDLK_RIGHT:d->mVelX += DOT_VEL; break;
		}
	}
	else if(e->type == SDL_KEYUP && e->key.repeat == 0)
	{
		switch(e->key.keysym.sym)
		{
			case SDLK_UP:	d->mVelY += DOT_VEL; break;
			case SDLK_DOWN:	d->mVelY -= DOT_VEL; break;
			case SDLK_LEFT: d->mVelX += DOT_VEL; break;
			case SDLK_RIGHT:d->mVelX -= DOT_VEL; break;
		}
	}
}

void Dot_move(Dot *d)
{
	d->mPosX += d->mVelX;

	if((d->mPosX < 0) || (d->mPosX + DOT_WIDTH > SCREEN_WIDTH))
		d->mPosX -= d->mVelX;

	d->mPosY += d->mVelY;

	if((d->mPosY < 0) || (d->mPosY + DOT_HEIGHT > SCREEN_HEIGHT))
		d->mPosY -= d->mVelY;
}

void Dot_render(Dot *d)
{
	LTexture_render(&gDotTexture, d->mPosX, d->mPosY, NULL);
}

short loadMedia()
{
	if(LTexture_loadFromFile(&gDotTexture, "dot.bmp") < 0)
		return -1;

	if(LTexture_loadFromFile(&gBGTexture, "bg.png") < 0)
		return -1;

	return 0;
}

void close_all()
{
	free_texture(&gDotTexture);
	free_texture(&gBGTexture);

	SDL_DestroyRenderer(gRenderer);
	SDL_DestroyWindow(gWindow);
	gWindow = NULL;
	gRenderer = NULL;

	IMG_Quit();
	SDL_Quit();
}

/*
 * Before we enter the main loop we declare a Dot object and the scrolling
 * offset.
 */
int main(int argc, char* argv[])
{
	if(init())
		goto equit;

	if(loadMedia())
		goto equit;

	SDL_Event e;

	Dot dot;
	int scrollingOffset = 0;

	while(1)
	{
		while(SDL_PollEvent( &e ) != 0) {
			if(e.type == SDL_QUIT)
				goto equit;

			Dot_handleEvent(&dot, &e);
		}
/*
 * Here we're updating the dot and updating the scrolling background.
 *
 * Updating the position of the scrolling background is just decrementing the x
 * position and if the x position is less than the width of the background that
 * means the background has gone completely off screen and the position needs
 * to be reset.
 */
		Dot_move(&dot);

		--scrollingOffset;
		if(scrollingOffset < -gBGTexture.mWidth)
			scrollingOffset = 0;
/*
 * Here we're rendering the background and the dot. First we render the
 * scrolling background by rendering two iterations of the texture next to each
 * other and then we render the dot over it. This will give us the effect of a
 * smooth scrolling infinite background.
 */
		SDL_SetRenderDrawColor(gRenderer, 0xFF, 0xFF, 0xFF, 0xFF);
		SDL_RenderClear(gRenderer);

		LTexture_render(&gBGTexture, scrollingOffset, 0, NULL);
		LTexture_render(
				&gBGTexture,
				scrollingOffset + gBGTexture.mWidth, 0, NULL);

		Dot_render(&dot);

		SDL_RenderPresent(gRenderer);
	}
equit:
	close_all();

	return 0;
}
