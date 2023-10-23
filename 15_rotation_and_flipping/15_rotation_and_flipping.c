/*
 * Rotation and Flipping
 *
 * SDL 2's hardware accelerated texture rendering also gives ability to give us
 * fast image flipping and rotation. In this tutorial we'll be using this to
 * make an arrow texture spin and flip.
 *
 * Here we're adding more functionality to LTexture_render. The render
 * function now takes in a rotation angle, a point to rotate the texture
 * around, and a SDL flipping enum.
 *
 * Like with clipping rectangles, we give the arguments default values in case
 * you want to render the texture without rotation or flipping.
 */
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>

#define SCREEN_WIDTH	640
#define SCREEN_HEIGHT	480

typedef struct {
	SDL_Texture *mTexture;
	int mWidth;
	int mHeight;
} LTexture;

typedef struct {
	SDL_RendererFlip flipType;
	double degrees;
} Data;

SDL_Window* gWindow = NULL;
SDL_Renderer* gRenderer = NULL;
LTexture gArrowTexture;

short init(void)
{
	if(SDL_Init(SDL_INIT_VIDEO) < 0) {
		SDL_Log("%s(), SDL_Init failed. %s", __func__, SDL_GetError());
		return -1;
	}

	if(SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "2") == 0)
		SDL_Log("Warning: Linear texture filtering not enabled.");

	gWindow = SDL_CreateWindow(
					"SDL Tutorial",
					SDL_WINDOWPOS_UNDEFINED,
					SDL_WINDOWPOS_UNDEFINED,
					SCREEN_WIDTH,
					SCREEN_HEIGHT,
					SDL_WINDOW_SHOWN );
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

short loadFromFile(LTexture *lt, char *path)
{
	free_texture(lt);

	SDL_Texture* newTexture = NULL;

	SDL_Surface* loadedSurface = IMG_Load(path);
	if(loadedSurface == NULL) {
		SDL_Log("%s(), IMG_Load failed. %s", __func__, IMG_GetError());

		return -1;
	}

	SDL_SetColorKey(loadedSurface, SDL_TRUE, SDL_MapRGB(
				loadedSurface->format, 0, 0xFF, 0xFF));

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

/*
 * As you can see, all we're doing is passing in the arguments from our
 * function to SDL_RenderCopyEx. This function works the same as the original
 * SDL_RenderCopy, but with additional arguments for rotation and flipping.
 */
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

short loadMedia(void)
{
	if(loadFromFile(&gArrowTexture, "arrow.png"))
		return -1;

	return 0;
}

void close_all(void)
{
	free_texture(&gArrowTexture);

	SDL_DestroyRenderer(gRenderer);
	SDL_DestroyWindow(gWindow);
	gWindow = NULL;
	gRenderer = NULL;

	IMG_Quit();
	SDL_Quit();
}

/*
 * In the event loop, we want to increment/decrement the rotation with the a/d
 * keys and change the type of flipping with the q,w, and e keys.
 */
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

/*
 * Before we enter the main loop we declare variables to keep track of the
 * rotation angle and flipping type.
 */
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
/*
 * Here we do the actual rendering. First we pass in the texture then the x and
 * y coordinates. That may seem like a complicated equation, but all it does
 * is center the image. If the image is 440 pixels wide on a 640 pixel wide
 * screen, we want it to be padded by 100 pixels on each side. In other words,
 * the x coordinate will be the screen width (640) minus the image width (440)
 * all divided by 2 ( (640 - 440 ) / 2 = 100).
 *
 * The next argument is the clip rectangle and since we're rendering the whole
 * texture it is set to null. The next argument is the rotation angle in
 * degrees. The next argument is the point we're rotation around. When this is
 * null, it will rotate around the center of the image. The last argument is
 * how the image flipped.
 *
 * The best way to wrap your mind around how to use rotation is to play around
 * with it. Experiment to see the type of effects you get by combining
 * different rotations/flipping.
 */
		LTexture_render(
			&gArrowTexture,
			(SCREEN_WIDTH - LTexture_getWidth(&gArrowTexture)) / 2,
			(SCREEN_HEIGHT - LTexture_getHeight(&gArrowTexture)) / 2,
			NULL,
			d.degrees,
			NULL,
			d.flipType);

		SDL_RenderPresent(gRenderer);
		SDL_Delay(60);
	}
equit:
	close_all();

	return 0;
}
