/*
 * Render to Texture
 *
 * For some effects being able to render a scene to texture is needed. Here
 * we'll be rendering a scene to a texture to achieve a spinning scene effect.
 */
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <stdio.h>

#define SCREEN_WIDTH	640
#define SCREEN_HEIGHT	480

/*
 * Here we are adding more functionality to the texture struct. The createBlank
 * function now takes in another argument that defines how it is accessed. We
 * also have the setAsRenderTarget function which makes it so we can render to
 * this texture.
 */
typedef struct {
	SDL_Texture* mTexture;
	void* mPixels;
	int mPitch;
	int mWidth;
	int mHeight;
} LTexture;

SDL_Window* gWindow = NULL;
SDL_Renderer* gRenderer = NULL;
LTexture gTargetTexture;

short init(void)
{
	if(SDL_Init(SDL_INIT_VIDEO) < 0) {
		SDL_Log("%s(), SDL_Init failed. %s", __func__, SDL_GetError());
		return -1;
	}

	if(SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "1") == 0) {
		SDL_Log("%s(), Warning: Linear texture filtering not enabled. %s",
				__func__, SDL_GetError());
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

void LTexture_free(LTexture *lt)
{
	if(lt->mTexture != NULL) {
		SDL_DestroyTexture(lt->mTexture);
		lt->mTexture = NULL;
		lt->mWidth = 0;
		lt->mHeight = 0;
		lt->mPixels = NULL;
		lt->mPitch = 0;
	}
}

short LTexture_loadFromFile(LTexture *lt, char *path)
{
	LTexture_free(lt);

	SDL_Texture* newTexture = NULL;

	SDL_Surface* loadedSurface = IMG_Load(path);
	if(loadedSurface == NULL) {
		SDL_Log("%s(), IMG_Load failed. %s", __func__, IMG_GetError());

		return -1;
	}

	SDL_Surface* formattedSurface = SDL_ConvertSurfaceFormat(
							loadedSurface,
							SDL_PIXELFORMAT_RGBA8888,
							SDL_SWSURFACE);
	if(formattedSurface == NULL) {
		SDL_Log("%s(), SDL_ConvertSurfaceFormat failed.", __func__);
		return -1;
	}

	newTexture = SDL_CreateTexture(
					gRenderer,
					SDL_PIXELFORMAT_RGBA8888,
					SDL_TEXTUREACCESS_STREAMING,
					formattedSurface->w,
					formattedSurface->h);
	if(newTexture == NULL) {
		SDL_Log("%s(), SDL_CreateTextureFromSurface failed. %s", __func__, SDL_GetError());
		return -1;
	}

	SDL_SetTextureBlendMode(newTexture, SDL_BLENDMODE_BLEND);

	SDL_LockTexture(
				newTexture,
				&formattedSurface->clip_rect,
				&lt->mPixels,
				&lt->mPitch);

	memcpy(
			lt->mPixels,
			formattedSurface->pixels,
			formattedSurface->pitch * formattedSurface->h);

	lt->mWidth = formattedSurface->w;
	lt->mHeight = formattedSurface->h;

	Uint32* pixels = (Uint32*)lt->mPixels;
	int pixelCount = (lt->mPitch / 4) * lt->mHeight;

	Uint32 colorKey = SDL_MapRGB(formattedSurface->format, 0, 0xFF, 0xFF);
	Uint32 transparent = SDL_MapRGBA(
			formattedSurface->format, 0x00, 0xFF, 0xFF, 0x00);

	int i;
	for(i = 0; i < pixelCount; ++i)
		if(pixels[i] == colorKey)
			pixels[i] = transparent;

	SDL_UnlockTexture(newTexture);
	lt->mPixels = NULL;

	SDL_FreeSurface(formattedSurface);
		
	SDL_FreeSurface(loadedSurface);

	lt->mTexture = newTexture;

	return 0;
}

/*
 * When we want to render to a texture we need to set its texture access to
 * SDL_TEXTUREACCESS_TARGET, which is why this function takes an additional
 * argument now.
 */
short LTexture_createBlank(
				LTexture *lt,
				int width,
				int height,
				SDL_TextureAccess access)
{
	lt->mTexture = SDL_CreateTexture(
						gRenderer,
						SDL_PIXELFORMAT_RGBA8888,
						access,
						width,
						height);
	if(lt->mTexture == NULL) {
		SDL_Log("%s(), SDL_CreateTexture failed.", __func__);
		return -1;
	}

	lt->mWidth = width;
	lt->mHeight = height;

	return 0;
}

void LTexture_render(
			LTexture *lt,
			int x, int y,
			SDL_Rect* clip,
			double angle,
			SDL_Point* center,
			SDL_RendererFlip flip)
{
	SDL_Rect renderQuad = { x, y, lt->mWidth, lt->mHeight };

	if(clip != NULL) {
		renderQuad.w = clip->w;
		renderQuad.h = clip->h;
	}

	SDL_RenderCopyEx(gRenderer, lt->mTexture, clip,
				&renderQuad, angle, center, flip);
}

/*
 * To render to a texture we have to set it as the render target which is done
 * here using a call to SDL_SetRenderTarget.
 */
void LTexture_setAsRenderTarget(LTexture *lt)
{
	SDL_SetRenderTarget(gRenderer, lt->mTexture);
}

/*
 * We create our target texture in the media loading function. 
 */
short loadMedia(void)
{
	if(LTexture_createBlank(
					&gTargetTexture,
					SCREEN_WIDTH,
					SCREEN_HEIGHT,
					SDL_TEXTUREACCESS_TARGET))
		return -1;

	return 0;
}

void close_all(void)
{
	LTexture_free(&gTargetTexture);

	SDL_DestroyRenderer(gRenderer);
	SDL_DestroyWindow(gWindow);
	gWindow = NULL;
	gRenderer = NULL;

	IMG_Quit();
	SDL_Quit();
}

/*
 * For this demo we'll render some geometry to a texture and spin that texture
 * around the center of the screen. This is why we have variables for angle of
 * rotation and center of screen.
 *
 * In our main loop before we do any rendering we set the target texture as a
 * target. We then render our scene full of geometry and once we're done
 * rendering to a texture we call SDL_SetRenderTarget with a NULL texture so
 * any rendering done afterward will be done to the screen.
 *
 * With our scene rendered to a texture, we then render the target texture to
 * the screen at a rotated angle.
 */
int main(int argc, char* args[])
{
	double angle = 0;
	SDL_Point screenCenter = { SCREEN_WIDTH / 2, SCREEN_HEIGHT / 2 };

	if(init())
		goto equit;

	if(loadMedia())
		goto equit;

	int i;
	SDL_Event e;

	while(1)
	{
		while(SDL_PollEvent(&e) != 0)
			if(e.type == SDL_QUIT)
				goto equit;

		LTexture_setAsRenderTarget(&gTargetTexture);

		SDL_SetRenderDrawColor(gRenderer, 0xFF, 0xFF, 0xFF, 0xFF);
		SDL_RenderClear(gRenderer);

		SDL_Rect fillRect = {
						SCREEN_WIDTH / 4,
						SCREEN_HEIGHT / 4,
						SCREEN_WIDTH / 2,
						SCREEN_HEIGHT / 2 };
		SDL_SetRenderDrawColor(gRenderer, 0xFF, 0x00, 0x00, 0xFF);		
		SDL_RenderFillRect(gRenderer, &fillRect);

		SDL_Rect outlineRect = {
						SCREEN_WIDTH / 6,
						SCREEN_HEIGHT / 6,
						SCREEN_WIDTH * 2 / 3,
						SCREEN_HEIGHT * 2 / 3 };
		SDL_SetRenderDrawColor(gRenderer, 0x00, 0xFF, 0x00, 0xFF);		
		SDL_RenderDrawRect(gRenderer, &outlineRect);
		
		SDL_SetRenderDrawColor(gRenderer, 0x00, 0x00, 0xFF, 0xFF);		
		SDL_RenderDrawLine(
					gRenderer,
					0,
					SCREEN_HEIGHT / 2, SCREEN_WIDTH,
					SCREEN_HEIGHT / 2);

		SDL_SetRenderDrawColor(gRenderer, 0xFF, 0xFF, 0x00, 0xFF);
		for(i = 0; i < SCREEN_HEIGHT; i += 4)
			SDL_RenderDrawPoint(gRenderer, SCREEN_WIDTH / 2, i);

		SDL_SetRenderTarget(gRenderer, NULL);
		LTexture_render(
					&gTargetTexture,
					0,
					0,
					NULL, 
					angle,
					&screenCenter,
					0);
		SDL_RenderPresent(gRenderer);

		if(angle > 360)
			angle = 0;
		else
			angle += 2;
	}
equit:
	close_all();

	return 0;
}

