/*
 * Animated Sprites and VSync
 *
 * Animation in a nutshell is just showing one image after another to create
 * the illusion of motion. Here we'll be showing different sprites to animate a
 * stick figure.
 *
 * Since images in SDL 2 are typically SDL_Textures, animating in SDL is a
 * matter of showing different parts of a texture (or different whole textures)
 * one right after the other.
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
 * So here we have the spritesheet with sprites that we're going to use for the
 * animation.
 */
#define WALKING_ANIMATION_FRAMES	4
SDL_Rect gSpriteClips[WALKING_ANIMATION_FRAMES];
LTexture gSpriteSheetTexture;

/*
 * For this (and future tutorials), we want to use Vertical Sync. VSync allows
 * the rendering to update at the same time as when your monitor updates during
 * vertical refresh. For this tutorial it will make sure the animation doesn't
 * run too fast. Most monitors run at about 60 frames per second and that's the
 * assumption we're making here. If you have a different monitor refresh rate,
 * that would explain why the animation is running too fast or slow.
 */
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
	if(gWindow == NULL) {
		SDL_Log("%s(), SDL_CreateWindow failed. %s", __func__, SDL_GetError());
		return -1;
	}

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
 * After we load the sprite sheet we want to define the sprites for the
 * individual frames of animation.
 */
short loadMedia(void)
{
	if(loadFromFile(&gSpriteSheetTexture, "foo.png"))
		return -1;

	gSpriteClips[0].x =   0;
	gSpriteClips[0].y =   0;
	gSpriteClips[0].w =  64;
	gSpriteClips[0].h = 205;

	gSpriteClips[1].x =  64;
	gSpriteClips[1].y =   0;
	gSpriteClips[1].w =  64;
	gSpriteClips[1].h = 205;

	gSpriteClips[2].x = 128;
	gSpriteClips[2].y =   0;
	gSpriteClips[2].w =  64;
	gSpriteClips[2].h = 205;

	gSpriteClips[3].x = 196;
	gSpriteClips[3].y =   0;
	gSpriteClips[3].w =  64;
	gSpriteClips[3].h = 205;

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

/*
 * Before the main loop we have to declare a variable to keep track of the
 * current frame of animation.
 *
 * After the screen is cleared in the main loop, we want to render the current
 * frame of animation. The animation goes from frames 0 to 3. Since there are
 * only 4 frames of animation, we want to slow down the animation a bit. This
 * is why when we get the current clip sprite, we want to divide the frame by
 * 4. This way the actual frame of animation only updates every 4 frames since
 * with int data types 0 / 4 = 0, 1 / 4 = 0, 2 / 4 = 0, 3 / 4 = 0, 4 / 4 = 1, 5
 * / 4 = 1, etc.
 *
 * After we get the current sprite, we want to render it to the screen and
 * then update it.
 *
 * After we update the frame by either incrementing it or cycling it back to 0,
 * we reach the end of the main loop. This main loop will keep showing a frame
 * and updating the animation value to animate the sprite.
 */
int main(int argc, char* argv[])
{
	int frame = 0;
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

		SDL_Rect* currentClip = &gSpriteClips[frame / 4];
		LTexture_render(
					&gSpriteSheetTexture,
					(SCREEN_WIDTH - currentClip->w) / 2,
					(SCREEN_HEIGHT - currentClip->h) / 2,
					currentClip);
		SDL_RenderPresent(gRenderer);
/*
 * In order for the frame to update, we need to increment the frame value
 * every frame. If we didn't, then the animation would stay at the first frame.
 *
 * We also want the animation to cycle, so when the frame hits the final value
 * ( 16 / 4 = 4 ) we reset the frame back to 0 so the animation starts over
 * again.
 */
		if(++frame / 4 >= WALKING_ANIMATION_FRAMES)
			frame = 0;

		SDL_Delay(30);
	}
equit:
	close_all();

	return 0;
}
