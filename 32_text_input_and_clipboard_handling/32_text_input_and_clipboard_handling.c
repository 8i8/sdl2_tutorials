/*
 * This program demonstrates the use of the clipboard for text input.
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

short init()
{
	if(SDL_Init(SDL_INIT_VIDEO) < 0) {
		SDL_Log("%s(), SDL_Init failed.", __func__);
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
		SDL_Log("%s(), SDL_CreateWindow failed.", __func__);
		return -1;
	}

	gRenderer = SDL_CreateRenderer(
					gWindow,
					-1,
					SDL_RENDERER_ACCELERATED
					| SDL_RENDERER_PRESENTVSYNC);
	if(gRenderer == NULL) {
		SDL_Log("%s(), SDL_CreateRenderer failed.", __func__);
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
		SDL_Log("%s(), TTF_RenderText_Solid failed.", __func__);
		return -1;
	}

	lt->mTexture = SDL_CreateTextureFromSurface(gRenderer, textSurface);
	if(lt->mTexture == NULL) {
		SDL_Log("%s(), SDL_CreateTextureFromSurface failed.",
				__func__);
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

short loadMedia()
{
	gFont = TTF_OpenFont("lazy.ttf", 28);
	if(gFont == NULL) {
		SDL_Log("%s(), TTF_OpenFont failed.", __func__);
		return -1;
	}

	SDL_Color textColor = { 0, 0, 0, 0xFF };
	if(LTexture_loadFromRenderedText(
						&gPromptTextTexture,
						"Enter Text:",
						textColor))
		return -1;

	return 0;
}

void close_all()
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
	else if(e->type == SDL_TEXTINPUT) {
		if(
					((e->text.text[0] == 'c'
					|| e->text.text[0] == 'C')
					&& (e->text.text[0] == 'v'
					|| e->text.text[0] == 'V')
					&& SDL_GetModState()
					& KMOD_CTRL)
					== 0)
		{
			strcat(inputText, e->text.text);
			return 1;
		}
	}
	return 0;
}

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

	while(1)
	{
		short renderText = 0;

		while(SDL_PollEvent(&e) != 0) {
			if(e.type == SDL_QUIT)
				goto equit;

			renderText = get_input(&e, inputText);
		}

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
