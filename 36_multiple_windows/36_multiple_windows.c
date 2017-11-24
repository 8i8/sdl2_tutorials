/*
 * This program demonstrated the use of multiple windows.
 */
#include <SDL2/SDL.h>
#include <stdio.h>

#define SCREEN_WIDTH	640
#define SCREEN_HEIGHT	480
#define TOTAL_WINDOWS	3

typedef struct {
	SDL_Window* mWindow;
	SDL_Renderer* mRenderer;
	unsigned mWindowID;

	int mWidth;
	int mHeight;

	short mMouseFocus;
	short mKeyboardFocus;
	short mFullScreen;
	short mMinimized;
	short mShown;
} LWindow;

LWindow gWindows[TOTAL_WINDOWS];

void LWindow_new(LWindow *w)
{
	w->mWindow = NULL;
	w->mRenderer = NULL;

	w->mMouseFocus = 0;
	w->mKeyboardFocus = 0;
	w->mFullScreen = 0;
	w->mShown = 0;
	w->mWindowID = -1;

	w->mWidth = 0;
	w->mHeight = 0;
}

short LWindow_init(LWindow *w)
{
	LWindow_new(w);

	w->mWindow = SDL_CreateWindow(
				"SDL Tutorial",
				SDL_WINDOWPOS_UNDEFINED,
				SDL_WINDOWPOS_UNDEFINED,
				SCREEN_WIDTH,
				SCREEN_HEIGHT,
				SDL_WINDOW_SHOWN
				| SDL_WINDOW_RESIZABLE);
	if(w->mWindow == NULL) {
		printf("Window could not be created! SDL Error: %s\n",
				SDL_GetError());
		return -1;
	}
	
	w->mMouseFocus = 1;
	w->mKeyboardFocus = 1;
	w->mWidth = SCREEN_WIDTH;
	w->mHeight = SCREEN_HEIGHT;

	w->mRenderer = SDL_CreateRenderer(
				w->mWindow,
				-1,
				SDL_RENDERER_ACCELERATED |
				SDL_RENDERER_PRESENTVSYNC);
	if(w->mRenderer == NULL) {
		SDL_Log("%s(), SDL_CreateRenderer failed. %s",
				__func__, SDL_GetError());
		SDL_DestroyWindow(w->mWindow);
		w->mWindow = NULL;
		return -1;
	}

	SDL_SetRenderDrawColor(w->mRenderer, 0xFF, 0xFF, 0xFF, 0xFF);
	w->mWindowID = SDL_GetWindowID(w->mWindow);
	w->mShown = 1;

	return 0;
}

short init()
{
	if(SDL_Init(SDL_INIT_VIDEO) < 0) {
		SDL_Log("%s(), SDL_Init failed. %s", __func__, SDL_GetError());
		return -1;
	}

	if(SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "1") == 0)
		SDL_Log("Warning: Linear texture filtering not enabled!");

	if(LWindow_init(&gWindows[0]))
		return -1;

	return 0;
}

void LWindow_handleEvent(LWindow *w, SDL_Event *e)
{
	if(e->type == SDL_WINDOWEVENT && e->window.windowID == w->mWindowID)
	{
		short updateCaption = 0;
		char caption[255];

		switch(e->window.event)
		{
			case SDL_WINDOWEVENT_SHOWN:
			//w->mShown = 1;
			break;

			case SDL_WINDOWEVENT_HIDDEN:
			w->mMinimized = 0;
			break;

			case SDL_WINDOWEVENT_SIZE_CHANGED:
			w->mWidth = e->window.data1;
			w->mHeight = e->window.data2;
			SDL_RenderPresent(w->mRenderer);
			break;

			case SDL_WINDOWEVENT_EXPOSED:
			SDL_RenderPresent(w->mRenderer);
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

			case SDL_WINDOWEVENT_CLOSE:
			SDL_HideWindow(w->mWindow);
			w->mShown = 0;
			break;
		}

		if(updateCaption) {
			sprintf(caption, "%s%u%s%s%s%s",
				"SDL Tutorial - ID: ",
				w->mWindowID-1,
				" MouseFocus:",
				((w->mMouseFocus) ? "On" : "Off"),
				" KeyboardFocus:",
				((w->mKeyboardFocus) ? "On" : "Off"));
			SDL_SetWindowTitle(w->mWindow, caption);
		}
	}
}

void LWindow_focus(LWindow *w)
{
	if(w->mShown == 0)
		SDL_ShowWindow(w->mWindow);

	SDL_RaiseWindow(w->mWindow);
}

void LWindow_render(LWindow *w)
{
	if(w->mMinimized == 0) {	
		SDL_SetRenderDrawColor(w->mRenderer, 0xFF, 0xFF, 0xFF, 0xFF);
		SDL_RenderClear(w->mRenderer);

		SDL_RenderPresent(w->mRenderer);
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

short LWindow_isShown(LWindow *w)
{
	return w->mShown;
}

void close_all()
{
	int i;
	for(i = 0; i < TOTAL_WINDOWS; ++i)
		LWindow_free(&gWindows[i]);

	SDL_Quit();
}

int main(int argc, char* argv[])
{
	int i;
	short allWindowsClosed;

	if(init())
		goto equit;

	for(i = 1; i < TOTAL_WINDOWS; ++i)
		LWindow_init(&gWindows[i]);

	SDL_Event e;

	while(1)
	{
		while(SDL_PollEvent(&e) != 0) {
			if(e.type == SDL_QUIT)
				goto equit;

			for(i = 0; i < TOTAL_WINDOWS; ++i)
				LWindow_handleEvent(&gWindows[i], &e);

			if(e.type == SDL_KEYDOWN)
			{
				switch(e.key.keysym.sym)
				{
					case SDLK_1:
					LWindow_focus(&gWindows[0]);
					break;

					case SDLK_2:
					LWindow_focus(&gWindows[1]);
					break;
						
					case SDLK_3:
					LWindow_focus(&gWindows[2]);
					break;

					case SDLK_q:
					goto equit;
					break;
				}
			}
		}

		for(i = 0; i < TOTAL_WINDOWS; ++i)
			LWindow_render(&gWindows[i]);
			
		allWindowsClosed = 1;
		for(i = 0; i < TOTAL_WINDOWS; ++i)
			if(LWindow_isShown(&gWindows[i])) {
				allWindowsClosed = 0;
				break;
			}

		if(allWindowsClosed)
			goto equit;

		SDL_Delay(16);
	}
equit:
	close_all();

	return 0;
}

