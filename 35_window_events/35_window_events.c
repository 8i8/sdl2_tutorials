/*
 * This program demonstrates the use of window events.
 */
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_ttf.h>
#include <stdio.h>

#define SCREEN_WIDTH	640
#define SCREEN_HEIGHT	480

typedef struct {
	SDL_Texture* mTexture;
	int mWidth;
	int mHeight;
} LTexture;

typedef struct {
	SDL_Window* mWindow;
	int mWidth;
	int mHeight;
	int mMouseFocus;
	int mKeyboardFocus;
	int mFullScreen;
	int mMinimized;
} LWindow;

LWindow gWindow;
SDL_Renderer* gRenderer = NULL;
LTexture gSceneTexture;

void LWindow_init(LWindow *w)
{
	w->mWindow = NULL;
	w->mMouseFocus = 1;
	w->mKeyboardFocus = 1;
	w->mFullScreen = 0;
	w->mMinimized = 0;
	w->mWidth = SCREEN_WIDTH;
	w->mHeight = SCREEN_HEIGHT;
}

short init()
{
	if(SDL_Init(SDL_INIT_VIDEO) < 0) {
		SDL_Log("%s(), SDL_Init failed. %s", __func__, SDL_GetError());
		return -1;
	}

	LWindow_init(&gWindow);

	gWindow.mWindow = SDL_CreateWindow(
				"SDL Tutorial",
				SDL_WINDOWPOS_UNDEFINED,
				SDL_WINDOWPOS_UNDEFINED,
				SCREEN_WIDTH,
				SCREEN_HEIGHT,
				SDL_WINDOW_SHOWN
				| SDL_WINDOW_RESIZABLE);
	if(gWindow.mWindow == NULL) {
		SDL_Log("%s(), SDL_CreateWindow failed.", __func__);
		return -1;
	}

	gRenderer = SDL_CreateRenderer(
				gWindow.mWindow,
				-1,
				SDL_RENDERER_ACCELERATED
				| SDL_RENDERER_PRESENTVSYNC);
	if(gRenderer == NULL) {
		SDL_Log("%s(), SDL_CreateRenderer failed.", __func__);
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

short LTexture_loadFromFile(LTexture *lt, char *path)
{
	free_texture(lt);

	SDL_Texture* newTexture = NULL;

	SDL_Surface* loadedSurface = IMG_Load(path);
	if(loadedSurface == NULL) {
		SDL_Log("%s(), IMG_Load failed to load \"%s\".",
				__func__, path);
		return -1;
	}

	SDL_SetColorKey(
			loadedSurface,
			SDL_TRUE,
			SDL_MapRGB(loadedSurface->format, 0, 0xFF, 0xFF));

	newTexture = SDL_CreateTextureFromSurface(gRenderer, loadedSurface);
	if(newTexture == NULL) {
		SDL_Log("%s(), SDL_CreateTextureFromSurface failed.",
				__func__);
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

void LWindow_handleEvent(LWindow *w, SDL_Event *e)
{
	if(e->type == SDL_WINDOWEVENT)
	{
		short updateCaption = 0;
		char caption[255];

		switch(e->window.event)
		{
			case SDL_WINDOWEVENT_SIZE_CHANGED:
			w->mWidth = e->window.data1;
			w->mHeight = e->window.data2;
			SDL_RenderPresent(gRenderer);
			break;

			case SDL_WINDOWEVENT_EXPOSED:
			SDL_RenderPresent(gRenderer);
			break;

			case SDL_WINDOWEVENT_ENTER:
			w->mMouseFocus = 1;
			updateCaption = 1;
			break;
			
			case SDL_WINDOWEVENT_LEAVE:
			w->mMouseFocus = 0;
			updateCaption = 1;
			break;

			case SDL_WINDOWEVENT_FOCUS_GAINED:
			w->mKeyboardFocus = 1;
			updateCaption = 1;
			break;

			case SDL_WINDOWEVENT_FOCUS_LOST:
			w->mKeyboardFocus = 0;
			updateCaption = 1;
			break;

			case SDL_WINDOWEVENT_MINIMIZED:
			w->mMinimized = 1;
			break;

			case SDL_WINDOWEVENT_MAXIMIZED:
			w->mMinimized = 0;
			break;
			
			case SDL_WINDOWEVENT_RESTORED:
			w->mMinimized = 0;
			break;
		}

		if(updateCaption) {
			sprintf(caption, "%s %s %s %s",
					"SDL Tutorial - MouseFocus:",
					((w->mMouseFocus) ? "On" : "Off"),
					" KeyboardFocus:",
					((w->mKeyboardFocus) ? "On" : "Off"));
			SDL_SetWindowTitle(w->mWindow, caption);
		}
	}
	else if(e->type == SDL_KEYDOWN && e->key.keysym.sym == SDLK_RETURN)
	{
		if(w->mFullScreen) {
			SDL_SetWindowFullscreen(w->mWindow, SDL_FALSE);
			w->mFullScreen = 0;
		}
		else {
			SDL_SetWindowFullscreen(w->mWindow, SDL_TRUE);
			w->mFullScreen = 1;
			w->mMinimized = 0;
		}
	}
}

void LWindow_free(LWindow *w)
{
	if(w->mWindow != NULL)
		SDL_DestroyWindow(w->mWindow);

	w->mMouseFocus = 0;
	w->mKeyboardFocus = 0;
	w->mWidth = 0;
	w->mHeight = 0;
}

short loadMedia()
{
	if(LTexture_loadFromFile(&gSceneTexture, "window.png") < 0) {
		printf("Failed to load window texture!\n");
		return -1;
	}

	return 0;
}

void close_all()
{
	free_texture(&gSceneTexture);

	SDL_DestroyRenderer(gRenderer);
	LWindow_free(&gWindow);

	IMG_Quit();
	SDL_Quit();
}

int main(int argc, char* argv[])
{
	if(init())
		goto equit;

	if(loadMedia())
		goto equit;

	SDL_Event e;

	while(1)
	{
		while(SDL_PollEvent(&e) != 0) {
			if(e.type == SDL_QUIT)
				goto equit;

			LWindow_handleEvent(&gWindow, &e);
		}

		if(gWindow.mMinimized == 0)
		{
			SDL_SetRenderDrawColor(gRenderer, 0xFF, 0xFF, 0xFF, 0xFF);
			SDL_RenderClear(gRenderer);

			LTexture_render(
					&gSceneTexture,
					(gWindow.mWidth - gSceneTexture.mWidth) / 2,
					(gWindow.mHeight - gSceneTexture.mHeight) / 2,
					NULL);

			SDL_RenderPresent(gRenderer);
		}
	}
equit:
	close_all();

	return 0;
}
