/*
 * Alpha Blending
 *
 * Thanks to new hardware accelerated rendering, transparency is much faster in
 * SDL 2.0. Here we'll use alpha modulation (which works much like color
 * modulation) to control the transparency of a texture.
 *
 * Here we're going to add two functions to support alpha transparency on a
 * texture. First there's LTexture_setAlpha which will function much like
 * LTexture_setColor did in the color modulation tutorial. There's also
 * LTexture_setBlendMode which will control how the texture is blended. In
 * order to get blending to work properly, you must set the blend mode on the
 * texture. We'll cover this in detail later.
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

SDL_Window* gWindow = NULL;
SDL_Renderer* gRenderer = NULL;
LTexture gModulatedTexture;
LTexture gBackgroundTexture;

short init(void)
{
	if(SDL_Init(SDL_INIT_VIDEO) < 0) {
		SDL_Log("%s(), SDL_Init failed. %s", __func__, SDL_GetError());
		return -1;
	}

	if(SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "1") == 0)
		SDL_Log("Warning: Linear texture filtering not enabled.");

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

	gRenderer = SDL_CreateRenderer(gWindow, -1, SDL_RENDERER_ACCELERATED);
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
 * Here are the SDL functions that do the actual work. SDL_SetTextureBlendMode
 * in setBlendMode allows us to enable blending and SDL_SetTextureAlphaMod
 * allows us to set the amount of alpha for the whole texture.
 */
void LTexture_setBlendMode(LTexture *lt, SDL_BlendMode blending)
{
	SDL_SetTextureBlendMode(lt->mTexture, blending);
}
		
void LTexture_setAlpha(LTexture *lt, Uint8 alpha)
{
	SDL_SetTextureAlphaMod(lt->mTexture, alpha);
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

int LTexture_getWidth(LTexture *lt)
{
	return lt->mWidth;
}

int LTexture_getHeight(LTexture *lt)
{
	return lt->mHeight;
}

/*
 * Here in the texture loading function we're loading the front texture we're
 * going to alpha blend and a background texture. As the front texture gets
 * more transparent, we'll be able to see more of the back texture. As you can
 * see in the code, after we load the front texture successfully we set the SDL
 * BlendMode to blend so blending is enabled. Since the background isn't going
 * to be transparent, we don't have to set the blending on it.
 * 
 * Now how does alpha work? Alpha is opacity, and the lower the opacity the
 * more we can see through it. Like red, green, or blue color components it
 * goes from 0 to 255 when modulating it. The best way to understand it is with
 * some examples. Say if we had the front image on a white background.
 * 
 * As will see that the lower the alpha the more transparent it is.
 */
short loadMedia(void)
{
	if(loadFromFile(&gModulatedTexture, "fadeout.png"))
		return -1;

	LTexture_setBlendMode(&gBackgroundTexture, SDL_BLENDMODE_BLEND);

	if(loadFromFile(&gBackgroundTexture, "fadein.png"))
		return -1;
	
	return 0;
}

void close_all(void)
{
	free_texture(&gModulatedTexture);
	free_texture(&gBackgroundTexture);

	SDL_DestroyRenderer(gRenderer);
	SDL_DestroyWindow(gWindow);
	gWindow = NULL;
	gRenderer = NULL;

	IMG_Quit();
	SDL_Quit();
}

/*
 * The event loop handles quit events and making the alpha value go up/down
 * with the w/s keys.
 */
void get_key_pressed(SDL_Event *e, Uint8 *a)
{
	if(e->key.keysym.sym == SDLK_w)
	{
		if(*a + 32 > 255)
			*a = 255;
		else
			*a += 32;
	}
	else if(e->key.keysym.sym == SDLK_s)
	{
		if(*a - 32 < 0)
			*a = 0;
		else
			*a -= 32;
	}
}

/*
 * Right before entering the main loop, we declare a variable to control how
 * much alpha the texture has. It is initialized to 255 so the front texture
 * starts out completely opaque.
 *
 * At the end of the main loop we do our rendering. After clearing the screen
 * we render the background first and then we render the front modulated
 * texture over it. Right before rendering the front texture, we set its alpha
 * value. Try increasing/decreasing the alpha value to see how transparency
 * affects the rendering.
 */
int main(int argc, char* argv[])
{
	SDL_Event e;
	SDL_Rect* clip = NULL;
	Uint8 a = 255;

	if(init())
		goto equit;

	if(loadMedia())
		goto equit;

	while(1)
	{
		while(SDL_PollEvent(&e) != 0)
		{
			if(e.type == SDL_QUIT)
				goto equit;

			if(e.type == SDL_KEYDOWN)
				get_key_pressed(&e, &a);
		}

		SDL_SetRenderDrawColor(gRenderer, 0xFF, 0xFF, 0xFF, 0xFF);
		SDL_RenderClear(gRenderer);

		LTexture_render(&gBackgroundTexture, 0, 0, clip);
		LTexture_setAlpha(&gModulatedTexture, a);
		LTexture_render(&gModulatedTexture, 0, 0, clip);

		SDL_RenderPresent(gRenderer);

		SDL_Delay(60);
	}
equit:
	close_all();

	return 0;
}

