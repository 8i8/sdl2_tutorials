/*
 * Advanced Timers
 *
 * Now that've made a basic timer with SDL, it's time to make one that can
 * start/stop/pause.
 *
 * For these new features, we're going to make a timer struct. It has all the
 * basic related function to start/stop/pause/unpause the timer and check its
 * status. In terms of data, we have the start time like before, a variable to
 * store the time when paused, and status flags to keep track of whether the
 * timer is running or paused.
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

LTexture gTimeTextTexture;
LTexture gPausePromptTexture;
LTexture gStartPromptTexture;

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
		SDL_Log("%s(), IMG_Init failed. %s",
				__func__, TTF_GetError());
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

/*
 * Our constructor initializes the internal data members. 
 */
void LTimer_init(LTimer *lt)
{
	lt->mStartTicks = 0;
	lt->mPausedTicks = 0;
	lt->mPaused = 0;
	lt->mStarted = 0;
}

/*
 * The start function sets the started and paused flags, gets the timer's start
 * time and initializes the pause time to 0. For this timer, if we want to
 * restart it we just call start again. Since we can start the timer if it is
 * paused and/or running, we should make sure to clear out the paused data.
 */
void LTimer_start(LTimer *lt)
{
	lt->mStarted = 1;
	lt->mPaused = 0;
	lt->mStartTicks = SDL_GetTicks();
	lt->mPausedTicks = 0;
}

/*
 * The stop function basically reinitializes all the variables.
 */
void LTimer_stop(LTimer *lt)
{
	lt->mStarted = 0;

	lt->mPaused = 0;

	lt->mStartTicks = 0;
	lt->mPausedTicks = 0;
}

/*
 * When pausing, we want to check if the timer is running because it doesn't
 * make sense to pause a timer that hasn't started. If the timer is running, we
 * set the pause flag, store the time when the timer was paused in
 * mPausedTicks, and reset the start time.
 */
void LTimer_pause(LTimer *lt)
{
	if(lt->mStarted && !lt->mPaused) {
		lt->mPaused = 1;
		lt->mPausedTicks = SDL_GetTicks() - lt->mStartTicks;
		lt->mStartTicks = 0;
	}
}

/*
 * So when we unpause the timer, we want to make sure the timer is running and
 * paused because we can't unpause a timer that's stopped or running. We set
 * the paused flag to false and set the new start time.
 *
 * Say if you start the timer when SDL_GetTicks() reports 5000 ms and then you
 * pause it at 10000ms. This means the relative time at the time of pausing is
 * 5000ms. If we were to unpause it when SDL_GetTicks was at 20000, the new
 * start time would be 20000 - 5000ms or 15000ms. This way the relative time
 * will still be 5000ms away from the current SDL_GetTicks time.
 */
void LTimer_unpause(LTimer *lt)
{
	if(lt->mStarted && lt->mPaused) {
		lt->mPaused = 0;
		lt->mStartTicks = SDL_GetTicks() - lt->mPausedTicks;
		lt->mPausedTicks = 0;
	}
}

/*
 * Getting the time is a little bit tricky since our timer can be running,
 * paused, or stopped. If the timer is stopped, we just return the initial 0
 * value. If the timer is paused, we return the time stored when paused. If the
 * timer is running and not paused, we return the time relative to when it
 * started.
 */
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

/*
 * Here we have some acccessor functions to check the status of the timer.
 */
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

	SDL_Color textColor = { 0, 0, 0, 255 };
	
	if(LTexture_loadFromRenderedText(
				&gStartPromptTexture,
				"Press S to Start or Stop the Timer",
				textColor))
		return -1;
	
	if(LTexture_loadFromRenderedText(
				&gPausePromptTexture,
				"Press P to Pause or Unpause the Timer",
				textColor))
		return -1;

	return 0;
}

void close_all(void)
{
	free_texture(&gTimeTextTexture);
	free_texture(&gStartPromptTexture);
	free_texture(&gPausePromptTexture);

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
 * When we press s key, we check if the timer is started. If it is, we stop it.
 * If it isn't, we start it. When we press p, we check if the timer is paused.
 * If it is, we unpause it. Otherwise we pause it.
 */
void get_key_event(SDL_Event *e, LTimer *timer)
{
	if(e->key.keysym.sym == SDLK_s)
	{
		if(LTimer_isStarted(timer))
			LTimer_stop(timer);
		else
			LTimer_start(timer);
	}
	else if(e->key.keysym.sym == SDLK_p)
	{
		if(LTimer_isPaused(timer))
			LTimer_unpause(timer);
		else
			LTimer_pause(timer);
	}
}

/*
 * Before we enter the main loop, we declare a timer object and a string stream
 * to turn the time value into text. 
 */
int main(int argc, char* argv[])
{
	SDL_Event e;
	SDL_Color textColor = { 0, 0, 0, 255 };
	char *text = "Milliseconds since start time ";
	int max_char_uint32 = 11;
	char timeText[strlen(text) + max_char_uint32];
	LTimer timer;

	if(init())
		goto equit;

	if(loadMedia())
		goto equit;
	
	LTimer_init(&timer);

	while(1)
	{
		while(SDL_PollEvent(&e) != 0)
		{
			if(e.type == SDL_QUIT)
				goto equit;

			if(e.type == SDL_KEYDOWN)
				get_key_event(&e, &timer);
		}
/*
 * Before we render, we write the current time to a string. The reason we
 * divide it by 1000 is because we want seconds and there are 1000 milliseconds
 * per second. After that we render the text to a texture and then finally draw
 * all the textures to the screen.
 */
		sprintf(timeText, "%s %6.4f", text,
				LTimer_getTicks(&timer) / 1000.f);

		if(LTexture_loadFromRenderedText(
					&gTimeTextTexture, timeText, textColor))
			goto equit;

		SDL_SetRenderDrawColor(gRenderer, 0xFF, 0xFF, 0xFF, 0xFF);
		SDL_RenderClear(gRenderer);

		LTexture_render(
				&gStartPromptTexture,
				(SCREEN_WIDTH - LTexture_getWidth(
						&gStartPromptTexture)) / 2,
				0,
				NULL, 0.0, NULL, SDL_FLIP_NONE);
		LTexture_render(
				&gPausePromptTexture,
				(SCREEN_WIDTH - LTexture_getWidth(
						&gPausePromptTexture)) / 2,
				LTexture_getHeight(&gStartPromptTexture),
				NULL, 0.0, NULL, SDL_FLIP_NONE);
		LTexture_render(
				&gTimeTextTexture,
				(SCREEN_WIDTH - LTexture_getWidth(
						&gTimeTextTexture)) / 2,
				(SCREEN_HEIGHT -LTexture_getHeight(
						&gTimeTextTexture)) / 2,
				NULL, 0.0, NULL, SDL_FLIP_NONE);

		SDL_RenderPresent(gRenderer);
	}
equit:
	close_all();

	return 0;
}

