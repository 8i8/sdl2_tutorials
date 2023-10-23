/*
 * Capping Frame Rate
 *
 * Another thing we can do with SDL timers is manually cap the frame rate. Here
 * we'll disable vsync and maintain a maximum frame rate.
 */
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_ttf.h>

/*
 * For this demo, we're going render our frame normally, but at the end of the
 * frame we're going to wait until the frame time is completed. For example
 * here, when you want to render at 60 fps you have to spend 16 and 2/3rd
 * milliseconds per frame ( 1000ms / 60 frames ). This is why here we calculate
 * the number of ticks per frame in milliseconds.
 */
#define SCREEN_WIDTH	640
#define SCREEN_HEIGHT	480
#define SCREEN_FPS		30
#define SCREEN_TICK_PER_FRAME	 1000 / SCREEN_FPS

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
/*
 * As you can see, we're disabling VSync for this demo because we'll be
 * manually capping the frame rate.
 */
	gRenderer = SDL_CreateRenderer(
					gWindow,
					-1,
					SDL_RENDERER_ACCELERATED);
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
		SDL_Log("%s(), IMG_Init failed. %s", __func__, TTF_GetError());
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
 * For this program we'll not only need a timer to calculate the frame rate,
 * but also a timer to cap the frames per second. Here, before we enter the main
 * loop, we declare some variables and start the fps calculator timer.
 */
int main(int argc, char* argv[])
{
	char *text = "Average Frames Per Second ";
	int max_char_uint32 = 11;
	char timeText[strlen(text) + max_char_uint32];

	if(init())
		goto equit;

	if(loadMedia())
		goto equit;

	SDL_Event e;

	SDL_Color textColor = { 0, 0, 0, 255 };
	LTimer fpsTimer;
	LTimer capTimer;

	int countedFrames = 0;
	float avgFPS;
	int frameTicks;

	LTimer_start(&fpsTimer);

/*
 * To cap the FPS we need to know how long the frame has taken to render which
 * is why we start a timer at the beginning of each frame.
 */
	while(1)
	{
		LTimer_start(&capTimer);
		while(SDL_PollEvent(&e) != 0)
			if(e.type == SDL_QUIT)
				goto equit;

		avgFPS = countedFrames / (LTimer_getTicks(&fpsTimer) / 1000.f);
		if(avgFPS > 2000000)
			avgFPS = 0;

		sprintf(timeText, "%s %7.4f", text, avgFPS);

		if(LTexture_loadFromRenderedText(
					&gFPSTextTexture, timeText, textColor))
			goto equit;

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
/*
 * Finally here we have the code to cap the frame rate. First we get how many
 * ticks the frame took to complete. If the number of ticks the frame took to
 * execute is less than the ticks needed per frame, we then delay for the
 * remaining time to prevent the application from running too fast.
 *
 * There's a reason we'll be using VSync for these tutorials as opposed to
 * manually capping the frame rate. When running this application, you'll
 * notice that it runs slightly fast. Since we're using integers (because
 * floating point numbers are not precise), the ticks per frame will be 16ms as
 * opposed to the exact 16 2/3ms. This solution is more of a stop gap in case
 * you have to deal with hardware that does not support VSync.
 */
		frameTicks = LTimer_getTicks(&capTimer);
		if(frameTicks < SCREEN_TICK_PER_FRAME)
			SDL_Delay(SCREEN_TICK_PER_FRAME - frameTicks);
	}
equit:
	close_all();

	return 0;
}

