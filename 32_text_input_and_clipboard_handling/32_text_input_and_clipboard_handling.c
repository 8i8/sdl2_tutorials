/*
 * Text Input And Clipboard Handling
 *
 * Getting text input from the keyboard is a common task in games. Here we'll
 * be getting text using SDL 2's new text input and clip board handling.
 */
#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <stdio.h>

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
LTexture gPromptTextTexture;
LTexture gInputTextTexture;

short init(void)
{
	if(SDL_Init(SDL_INIT_VIDEO) < 0) {
		SDL_Log("%s(), SDL_Init failed. %s", __func__, SDL_GetError());
		return -1;
	}

	if(SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "4") == 0) {
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
	if(gRenderer == NULL) {
		SDL_Log("%s(), SDL_CreateRenderer failed. %s", __func__, SDL_GetError());
		return -1;
	}

	SDL_SetRenderDrawColor(gRenderer, 0xFF, 0xFF, 0xFF, 0xFF);

	if(TTF_Init() == -1) {
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

short LTexture_render(LTexture *lt, int x, int y, SDL_Rect* clip)
{
	SDL_Rect renderQuad = {x, y, lt->mWidth, lt->mHeight};

	if(clip != NULL) {
		renderQuad.w = clip->w;
		renderQuad.h = clip->h;
	}

	return SDL_RenderCopy(gRenderer, lt->mTexture, clip, &renderQuad);
}

short loadMedia(void)
{
	//gFont = TTF_OpenFont("lazy.ttf", 28);
	gFont = TTF_OpenFont("DejaVuSans.ttf", 28);
	if(gFont == NULL) {
		SDL_Log("%s(), TTF_OpenFont failed. %s", __func__, TTF_GetError());
		return -1;
	}

	SDL_Color textColor = { 0, 0, 0, 0xFF };
	if(LTexture_loadFromRenderedText(&gPromptTextTexture,
						"Enter Text:", textColor))
		return -1;

	return 0;
}

/*
 * Once we're done with text input we disable it since enabling text input
 * introduces some overhead.
 */
void close_all(void)
{
	free_texture(&gPromptTextTexture);
	free_texture(&gInputTextTexture);

	TTF_CloseFont(gFont);
	gFont = NULL;

	SDL_DestroyRenderer(gRenderer);
	SDL_DestroyWindow(gWindow);
	gWindow = NULL;
	gRenderer = NULL;

	TTF_Quit();
	SDL_Quit();
}

/*
 * There are a couple special key presses we want to handle. When the user
 * presses back space we want to remove the last character from the string.
 *
 * When the user is holding control and presses c, we want to copy the current
 * text to the clip board using SDL_SetClipboardText. You can check if the ctrl
 * key is being held using SDL_GetModState.
 *
 * When the user does ctrl + v, we want to get the text from the clip board
 * using SDL_GetClipboardText. Also notice that whenever we alter the contents
 * of the string we set the text update flag.
 */
short get_input(SDL_Event *e, char *inputText)
{
	int len = 0;
	len = strlen(inputText);

	if(e->type == SDL_KEYDOWN)
	{
		if(e->key.keysym.sym == SDLK_BACKSPACE && len > 0) {
			*(inputText+len-1) = '\0';
			return 1;
		}
		else if(e->key.keysym.sym ==
				SDLK_c && SDL_GetModState() & KMOD_CTRL) {
			SDL_SetClipboardText(inputText);
		}
		else if(e->key.keysym.sym ==
				SDLK_v && SDL_GetModState() & KMOD_CTRL) {
			strcpy(inputText, SDL_GetClipboardText());
			return 1;
		}
	}
/*
 * With text input enabled, your key presses will also generate
 * SDL_TextInputEvents which simplifies things like shift key and caps lock.
 * Here we first want to check that we're not getting a ctrl and c/v event
 * because we want to ignore those since they are already handled as keydown
 * events. If it isn't a copy or paste event, we append the character to our
 * input string.
 */
	else if(e->type == SDL_TEXTINPUT) {
		if(((e->text.text[0] == 'c' || e->text.text[0] == 'C')
					&& (e->text.text[0] == 'v' || e->text.text[0] == 'V')
					&& SDL_GetModState() & KMOD_CTRL) == 0)
		{
			strcat(inputText, e->text.text);
			return 1;
		}
	}
	return 0;
}

/*
 * Before we go into the main loop we declare a string to hold our text and
 * render it to a texture. We then call SDL_StartTextInput so the SDL text
 * th text input enabled, your key presses will also generate
 * SDL_TextInputEvents which simplifies things like shift key and caps lock.
 * Here we first want to check that we're not getting a ctrl and c/v event
 * because we want to ignore those since they are already handled as keydown
 * events. If it isn't a copy or paste event, we append the character to our
 * input string.input functionality is enabled.
 */
int main(int argc, char* argv[])
{
	if(init())
		goto equit;

	if(loadMedia())
		goto equit;

	SDL_Event e;
	SDL_Color textColor = { 0, 0, 0, 0xFF };

	char inputText[255] = "Some Text";
	LTexture_loadFromRenderedText(
			&gInputTextTexture, inputText, textColor);

	SDL_StartTextInput();
/*
 * We only want to update the input text texture when we need to so we have a
 * flag that keeps track of whether we need to update the texture.
 */
	while(1)
	{
		short renderText = 0;

		while(SDL_PollEvent(&e) != 0) {
			if(e.type == SDL_QUIT)
				goto equit;

			renderText = get_input(&e, inputText);
		}
/*
 * If the text render update flag has been set, we rerender the texture. One
 * little hack we have here is if we have an empty string, we render a space
 * because SDL_ttf will not render an empty string.
 * At the end of the main loop we render the prompt text and the input text.
 */
		if(renderText) {
			if(strcmp(inputText, ""))
				LTexture_loadFromRenderedText(
							&gInputTextTexture,
							inputText,
							textColor);
			else
				LTexture_loadFromRenderedText(
							&gInputTextTexture,
							" ",
							textColor);
		}

		SDL_SetRenderDrawColor(gRenderer, 0xFF, 0xFF, 0xFF, 0xFF);
		SDL_RenderClear(gRenderer);

		LTexture_render(
				&gPromptTextTexture,
				(SCREEN_WIDTH - gPromptTextTexture.mWidth) / 2,
				0,
				NULL);
		LTexture_render(
				&gInputTextTexture,
				(SCREEN_WIDTH - gInputTextTexture.mWidth) / 2,
				gPromptTextTexture.mHeight,
				NULL);

		SDL_RenderPresent(gRenderer);
	}
	
	SDL_StopTextInput();
equit:
	close_all();

	return 0;
}
