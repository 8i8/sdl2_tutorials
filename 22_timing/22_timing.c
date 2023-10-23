/*
 * Timing
 *
 * Another important part of any sort of gaming API is the ability to handle
 * time. In this tutorial we'll make a timer we can restart.
 */
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_ttf.h>

#define SCREEN_WIDTH	640
#define SCREEN_HEIGHT	480

typedef struct {
	SDL_Texture *mTexture;
	int mWidth;
	int mHeight;
} LTexture;

SDL_Window* gWindow = NULL;
SDL_Renderer* gRenderer = NULL;
TTF_Font *gFont = NULL;

LTexture gTimeTextTexture;
LTexture gPromptTextTexture;

short init(void)
{
	if(SDL_Init(SDL_INIT_VIDEO) < 0) {
		SDL_Log("%s(), SDL_Init failed. %s", __func__, SDL_GetError());
		return -1;
	}

	if(SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "1") == 0)
		SDL_Log("Warning: Linear texture filtering disabled.");


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

short LTexture_loadFromFile(LTexture *lt, char *path)
{
	free_texture(lt);

	SDL_Texture* newTexture = NULL;

	SDL_Surface* loadedSurface = IMG_Load(path);
	if(loadedSurface == NULL) {
		SDL_Log("%s(), IMG_Load failed. %s", __func__, IMG_GetError());

		return -1;
	}

	SDL_SetColorKey(
			loadedSurface,
			SDL_TRUE,
			SDL_MapRGB(loadedSurface->format, 0, 0xFF, 0xFF));

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
 * As mentioned in the font rendering tutorial, you want to minimize the amount
 * of times you render text. We'll have a texture to prompt input and a texture
 * to display the current time in milliseconds. The time texture changes every
 * frame so we have to render that every frame, but the prompt texture doesn't
 * change so we can render it once in the file loading function.
 */
short loadMedia(void)
{
	gFont = TTF_OpenFont("lazy.ttf", 28);
	if(gFont == NULL) {
		SDL_Log("%s(), TTF_OpenFont failed. %s",
				__func__, TTF_GetError());
		return -1;
	}

	SDL_Color textColor = { 0, 0, 0, 255 };
	
	if(LTexture_loadFromRenderedText(
					&gPromptTextTexture,
					"Press Enter to Reset Start Time.",
					textColor))
		return -1;

	return 0;
}

void close_all(void)
{
	free_texture(&gTimeTextTexture);
	free_texture(&gPromptTextTexture);

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
 * Before we enter the main loop we want to declare some variables. The two we
 * want to pay attention to is the startTime variable (which is an Unsigned
 * integer that's 32bits) and the timeText variable which is a string stream.
 *
 * For those of you who have never used string streams, just know that they
 * function like iostreams only instead of reading or writing to the console,
 * they allow you to read and write to a string in memory. It'll be easier to
 * see when we see them used further on in the program.
 */
int main(int argc, char *argv[])
{
	SDL_Event e;
	char *text = "Milliseconds since start time ";
	int max_char_uint32 = 11;
	char timeText[strlen(text) + max_char_uint32];
	Uint32 startTime = 0;
			
	if(init())
		goto equit;

	if(loadMedia())
		goto equit;

	SDL_Color textColor = { 0, 0, 0, 255 };

	while(1)
	{
		while(SDL_PollEvent(&e) != 0)
		{
			if(e.type == SDL_QUIT)
				goto equit;

			if(
					e.type == SDL_KEYDOWN
					&& e.key.keysym.sym == SDLK_RETURN)
				startTime = SDL_GetTicks();
		}
/*
 * There's a function called SDL_GetTicks which returns the time since the
 * program started in milliseconds. For this demo, we'll be having a timer that
 * restarts every time we press the return key.
 *
 * Remember how we initialized the start time to 0 at the start of the program?
 * This means the timer's time is just the current time since the program
 * started returned by SDL_GetTicks. If we were to restart the timer when
 * SDL_GetTicks was at 5000 milliseconds (5 seconds), then at 10,000
 * milliseconds the current time - the start time would be 10000 minus 5000
 * would be 5000 milliseconds. So even though the timer contained by
 * SDL_GetTicks hasn't restarted, we can have a timer keep track of a relative
 * start time and reset its start time. 
 */
/*
 * Here we're using sprintf to fill timeText witf 'time' text and then the time
 * since the timer started minus the currnet ticksvalue.
 */
		sprintf(timeText, "%s %u", text, SDL_GetTicks() - startTime);

		if(LTexture_loadFromRenderedText(
					&gTimeTextTexture, timeText, textColor))
			goto equit;

		SDL_SetRenderDrawColor(gRenderer, 0xFF, 0xFF, 0xFF, 0xFF);
		SDL_RenderClear(gRenderer);

		LTexture_render(
				&gPromptTextTexture,
				(SCREEN_WIDTH - LTexture_getWidth(
						&gPromptTextTexture)) / 2,
				0, NULL, 0.0, NULL, SDL_FLIP_NONE);
/*
 * Now that we have the time in a string, we can use it to render the current
 * time to a texture.
 */
		LTexture_render(
				&gTimeTextTexture,
				(SCREEN_WIDTH - LTexture_getWidth(
						&gPromptTextTexture)) / 2,
				(SCREEN_HEIGHT - LTexture_getHeight(
						&gPromptTextTexture)) / 2,
				NULL, 0.0, NULL, SDL_FLIP_NONE);
/*
 * Finally we render the prompt texture and the time texture to the screen.
 */
		SDL_RenderPresent(gRenderer);
	}
equit:
	close_all();

	return 0;
}

