/*
 * Calculating Frame Rate
 *
 * Now that we know how to make a timer with SDL it's time to put it to use.
 * We're going to use the timer to measure fps.
 */
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_ttf.h>

const int SCREEN_WIDTH = 640;
const int SCREEN_HEIGHT = 480;

typedef struct {
	SDL_Texture *mTexture;
	int mWidth;
	int mHeight;
} LTexture;

typedef struct {
	Uint32 mStartTicks;
	Uint32 mPausedTicks;
	short mPaused;
	short mStarted;
} LTimer;

SDL_Window* gWindow = NULL;
SDL_Renderer* gRenderer = NULL;
TTF_Font* gFont = NULL;
LTexture gFPSTextTexture;

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

	if(TTF_Init() < 0) {
		SDL_Log("%s(), TTF_Init failed. %s", __func__, TTF_GetError());
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

short LTexture_loadFromRenderedText(
					LTexture *lt,
					char *textureText,
					SDL_Color textColor)
{
	free_texture(lt);

	SDL_Surface* textSurface = TTF_RenderText_Solid(
						gFont, textureText, textColor);
	if(textSurface == NULL) {
		SDL_Log("%s(), TTF_RenderText_Solid failed. %s", __func__, TTF_GetError());
		return -1;
	}

	lt->mTexture = SDL_CreateTextureFromSurface(gRenderer, textSurface);
	if(lt->mTexture == NULL) {
		SDL_Log("%s(), SDL_CreateTextureFromSurface failed. %s", __func__, SDL_GetError());

		return -1;
	}

	lt->mWidth = textSurface->w;
	lt->mHeight = textSurface->h;

	SDL_FreeSurface(textSurface);
	
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

	if(clip != NULL) {
		renderQuad.w = clip->w;
		renderQuad.h = clip->h;
	}

	return SDL_RenderCopyEx(gRenderer, lt->mTexture, clip,
				&renderQuad, angle, center, flip);
}

int LTexture_getWidth(LTexture *lt)
{
	return lt->mWidth;
}

int LTexture_getHeight(LTexture *lt)
{
	return lt->mHeight;
}

void LTimer_init(LTimer *lt)
{
	lt->mStartTicks = 0;
	lt->mPausedTicks = 0;
	lt->mPaused = 0;
	lt->mStarted = 0;
}

void LTimer_start(LTimer *lt)
{
	lt->mStarted = 1;
	lt->mPaused = 0;
	lt->mStartTicks = SDL_GetTicks();
	lt->mPausedTicks = 0;
}

Uint32 LTimer_getTicks(LTimer *lt)
{
	if(lt->mStarted) {
		if(lt->mPaused)
			return lt->mPausedTicks;
		else
			return SDL_GetTicks() - lt->mStartTicks;
	}
	return 0;
}

short LTimer_isStarted(LTimer *lt)
{
	return lt->mStarted;
}

short LTimer_isPaused(LTimer *lt)
{
	return lt->mPaused && lt->mStarted;
}

short loadMedia(void)
{
	gFont = TTF_OpenFont("lazy.ttf", 28);
	if(gFont == NULL) {
		SDL_Log("%s(), TTF_OpenFont failed. %s", __func__, TTF_GetError());
		return -1;
	}

	return 0;
}

void close_all(void)
{
	free_texture(&gFPSTextTexture);

	TTF_CloseFont(gFont);
	gFont = NULL;

	SDL_DestroyRenderer(gRenderer);
	SDL_DestroyWindow(gWindow);
	gWindow = NULL;
	gRenderer = NULL;

	TTF_Quit();
	IMG_Quit();
	SDL_Quit();
}

/*
 * In order to calculate the frames per second, we need to keep track of the
 * number of frames rendered and the number of second passed. Before we enter
 * the main loop, we start a timer used to calculate fps and declare a variable
 * to keep track of the number of frames rendered.
 */
int main(int argc, char* args[])
{
	char *text = "Average Frames Per Second ";
	int max_char_uint32 = 11;
	char timeText[strlen(text) + max_char_uint32];
	int countedFrames = 0;
	float avgFPS;

	if(init())
		goto equit;

	if(loadMedia())
		goto equit;

	SDL_Event e;
	SDL_Color textColor = { 0, 0, 0, 255 };
	LTimer fpsTimer;

	LTimer_init(&fpsTimer);
	LTimer_start(&fpsTimer);

	while(1)
	{
		while(SDL_PollEvent( &e ) != 0)
			if(e.type == SDL_QUIT)
				goto equit;
/*
 * To calculate frames per second, you just take the number of rendered frames
 * and divide it by the seconds passed. Now it is possible for there to be a
 * very small amount of time passed for the first frame and have it give us a
 * really high fps. This is why we correct the value if it is really high.
 */
		avgFPS = countedFrames / (LTimer_getTicks(&fpsTimer) / 1000.f);
		if(avgFPS > 2000000)
			avgFPS = 0;
		
		sprintf(timeText, "%s %6.4f", text, avgFPS);

		if(LTexture_loadFromRenderedText(
					&gFPSTextTexture, timeText, textColor))
			goto equit;
/*
 * After calculating the fps, we render the value as a texture to the screen.
 * After we're done rendering the scene, we increment the frame counter.
 *
 * Since this program is vsynced, it is probably going to report 60 fps. If you
 * want to find out how much you hardware can do, just create a renderer
 * without vsync.
 */
		SDL_SetRenderDrawColor(gRenderer, 0xFF, 0xFF, 0xFF, 0xFF);
		SDL_RenderClear(gRenderer);

		LTexture_render(
				&gFPSTextTexture,
				(SCREEN_WIDTH - LTexture_getWidth(
							&gFPSTextTexture)) / 2,
				(SCREEN_HEIGHT - LTexture_getHeight(
							&gFPSTextTexture)) / 2,
				NULL, 0.0, NULL, SDL_FLIP_NONE);

		SDL_RenderPresent(gRenderer);
		++countedFrames;
	}
equit:
	close_all();

	return 0;
}

