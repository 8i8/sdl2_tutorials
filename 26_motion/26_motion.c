/*
 * Motion
 *
 * Now that we know how to render, handle input, and deal with time we can now
 * know everything we need move around things on the screen. Here will do a
 * basic dot moving around.
 */
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <stdio.h>

#define SCREEN_WIDTH	640
#define SCREEN_HEIGHT	480
#define DOT_WIDTH	20
#define DOT_HEIGHT	20
#define DOT_VEL		5

typedef struct {
	SDL_Texture *mTexture;
	int mWidth;
	int mHeight;
} LTexture;

/*
 * Here is the struct for the dot we're going to be moving around on the
 * screen.  It has some constants to define its dimensions and velocity. It has
 * an a related event handler, a function to move it every frame, and a
 * function to render it. As for data members, it has variables for its x/y
 * position and x/y velocity.
 */
typedef struct {
	int mPosX, mPosY;
	int mVelX, mVelY;
} Dot;

SDL_Window* gWindow = NULL;
SDL_Renderer* gRenderer = NULL;
LTexture gDotTexture;

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
					SDL_RENDERER_ACCELERATED);
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

/*
 * In the rendering function we render the dot texture at the dot's position. 
 */
short LTexture_render(LTexture *lt, int x, int y, SDL_Rect* clip)
{
	SDL_Rect renderQuad = {x, y, lt->mWidth, lt->mHeight};

	if(clip != NULL) {
		renderQuad.w = clip->w;
		renderQuad.h = clip->h;
	}

	return SDL_RenderCopy(gRenderer, lt->mTexture, clip, &renderQuad);
}

/*
 * The constructor simply initializes the variables.
 */
Dot *Dot_init(Dot *d)
{
	d->mPosX = 0;
	d->mPosY = 0;
	d->mVelX = 0;
	d->mVelY = 0;
	return d;
}

/*
 * In our event handler we're going to set the velocity based on the key press.
 *
 * You may be wondering why we don't simply just increase the positon when we
 * press the key. If we were to say add to the x position every time we press
 * the right key, we would have to repeatedly press the right key to keep it
 * moving. By setting the velocity, we just have to press the key once.
 *
 * If you're wondering why we're checking if the key repeat is 0, it's because
 * key repeat is enabled by default and if you press and hold a key it will
 * report multiple key presses. That means we have to check if the key press is
 * the first one because we only care when the key was first pressed.
 *
 * For those of you who haven't studied physics yet, velocity is the
 * speed/direction of an object. If an object is moving right at 10 pixels per
 * frame, it has a velocity of 10. If it is moving to the left at 10 pixel per
 * frame, it has a velocity of -10. If the dot's velocity is 10, this means
 * after 10 frames it will have moved 100 pixels over.
 */
void Dot_handleEvent(SDL_Event *e, Dot *d)
{
	if(e->type == SDL_KEYDOWN && e->key.repeat == 0)
	{
		switch(e->key.keysym.sym)
		{
			case SDLK_UP: d->mVelY -= DOT_VEL; break;
			case SDLK_DOWN: d->mVelY += DOT_VEL; break;
			case SDLK_LEFT: d->mVelX -= DOT_VEL; break;
			case SDLK_RIGHT: d->mVelX += DOT_VEL; break;
		}
	}
/*
 * When we release a key, we have to undo the velocity change when first
 * pressed it. When we pressed right key, we added to the x velocity. When we
 * release the right key here, we subtract from the x velocity to return it to
 * 0.
 */
	else if(e->type == SDL_KEYUP && e->key.repeat == 0)
	{
		switch(e->key.keysym.sym)
		{
			case SDLK_UP: d->mVelY += DOT_VEL; break;
			case SDLK_DOWN:d-> mVelY -= DOT_VEL; break;
			case SDLK_LEFT: d->mVelX += DOT_VEL; break;
			case SDLK_RIGHT: d->mVelX -= DOT_VEL; break;
		}
	}
}

/*
 * Here's the function we call every frame to move the dot.
 *
 * First we move the dot along the x axis based on its x velocity. After that
 * we check if the dot moved off the screen. If it did, we then undo the
 * movement along the x axis.
 */
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
	if(LTexture_loadFromFile(&gDotTexture, "dot.bmp"))
		return -1;
	return 0;
}

void close_all()
{
	free_texture(&gDotTexture);

	SDL_DestroyRenderer(gRenderer);
	SDL_DestroyWindow(gWindow);
	gWindow = NULL;
	gRenderer = NULL;

	IMG_Quit();
	SDL_Quit();
}

/*
 * Before we enter the main loop we declare a dot object.
 *
 * Finally here we use our dot in the main loop. In the event loop we handle
 * events for the dot. After that we update the dot's position and then render
 * it to the screen.
 *
 * Now in this tutorial we're basing the velocity as amount moved per frame. In
 * most games, the velocity is done per second. The reason were doing it per
 * frame is that it is easier, but if you know physics it shouldn't be hard to
 * update the dot's position based on time. If you don't know physics, just
 * stick with per frame velocity for now.
 */
int main(int argc, char* args[])
{
	if(init())
		goto equit;

	if(loadMedia())
		goto equit;

	SDL_Event e;
	Dot dot;
	Dot_init(&dot);

	while(1)
	{
		while(SDL_PollEvent(&e) != 0) {
			if(e.type == SDL_QUIT)
				goto equit;

			Dot_handleEvent(&e, &dot);
		}

		Dot_move(&dot);

		SDL_SetRenderDrawColor(gRenderer, 0xFF, 0xFF, 0xFF, 0xFF);
		SDL_RenderClear(gRenderer);

		Dot_render(&dot);

		SDL_RenderPresent(gRenderer);
		SDL_Delay(16);
	}
equit:
	close_all();

	return 0;
}
