/*
 * Clip Rendering and Sprite Sheets
 *
 * Sometimes you only want to render part of a texture. A lot of times games
 * like to keep multiple images on the same sprite sheet as opposed to having a
 * bunch of textures. Using clip rendering, we can define a portion of the
 * texture to render as opposed to rendering the whole thing. 
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

/*
 * For this tutorial, we're going to take this sprite sheet and render each
 * sprite in a different corner; So we're going to need a texture image and 4
 * rectangles to define the sprites, which are the variables you see declared
 * here.
 */
SDL_Rect gSpriteClips[4];
LTexture gSpriteSheetTexture;

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

LTexture *LTexture_init(LTexture *lt)
{
	lt->mTexture = NULL;
	lt->mWidth = 0;
	lt->mHeight = 0;
	return lt;
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
 * Here is the new rendering function for the texture class that supports clip
 * rendering. It's mostly the same as the previous texture rendering function
 * but with two changes.
 * First, since when you're clipping and you're using the dimensions of the
 * clip rectangle instead of the texture, we're going to set the width/height
 * of the destination rectangle (here called renderQuad) to the size of the
 * clip rectangle.
 * Secondly, we're going to pass in the clip rectangle to SDL_RenderCopy as the
 * source rectangle. The source rectangle defines what part of the texture you
 * want to render. When the source rectangle is NULL, the whole texture is
 * rendered.
 */
short render(LTexture *lt, int x, int y, SDL_Rect* clip)
{
	SDL_Rect renderQuad = {x, y, lt->mWidth, lt->mHeight};

	if(clip != NULL) {
		renderQuad.w = clip->w;
		renderQuad.h = clip->h;
	}

	return SDL_RenderCopy(gRenderer, lt->mTexture, clip, &renderQuad);
}

/*
 * The media loading function loads the texture and then defines the clip
 * rectangles for the circle sprites if the texture loaded successfully.
 */
short loadMedia(void)
{
	if(loadFromFile(&gSpriteSheetTexture, "dots.png"))
		return -1;

	gSpriteClips[0].x =   0;
	gSpriteClips[0].y =   0;
	gSpriteClips[0].w = 100;
	gSpriteClips[0].h = 100;

	gSpriteClips[1].x = 100;
	gSpriteClips[1].y =   0;
	gSpriteClips[1].w = 100;
	gSpriteClips[1].h = 100;

	gSpriteClips[2].x =   0;
	gSpriteClips[2].y = 100;
	gSpriteClips[2].w = 100;
	gSpriteClips[2].h = 100;

	gSpriteClips[3].x = 100;
	gSpriteClips[3].y = 100;
	gSpriteClips[3].w = 100;
	gSpriteClips[3].h = 100;

	return 0;
}

void close_all(void)
{
	free_texture(&gSpriteSheetTexture);

	SDL_DestroyRenderer(gRenderer);
	SDL_DestroyWindow(gWindow);
	gWindow = NULL;
	gRenderer = NULL;

	IMG_Quit();
	SDL_Quit();
}

int main(int argc, char* argv[])
{
	SDL_Event e;

	if(init())
		goto equit;

	if(loadMedia())
		goto equit;

	while(1)
	{
		while(SDL_PollEvent(&e) != 0)
			if(e.type == SDL_QUIT)
				goto equit;

		SDL_SetRenderDrawColor(gRenderer, 0xFF, 0xFF, 0xFF, 0xFF);
		SDL_RenderClear(gRenderer);
/*
 * Finally here in the main loop we render the same texture 4 times, but we're
 * rendering a different portion of the sprite sheet in different places each
 * call. 
 */
		render(
				&gSpriteSheetTexture,
				0,
				0,
				&gSpriteClips[0]);

		render(
				&gSpriteSheetTexture,
				SCREEN_WIDTH - gSpriteClips[1].w,
				0,
				&gSpriteClips[1]);

		render(
				&gSpriteSheetTexture,
				0,
				SCREEN_HEIGHT - gSpriteClips[2].h,
				&gSpriteClips[2]);

		render(
				&gSpriteSheetTexture,
				SCREEN_WIDTH - gSpriteClips[3].w,
				SCREEN_HEIGHT - gSpriteClips[3].h,
				&gSpriteClips[3]);

		SDL_RenderPresent(gRenderer);
		SDL_Delay(60);
	}
equit:
	close_all();

	return 0;
}

