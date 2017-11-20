#include <SDL2/SDL.h>
#include <stdio.h>

const int SCREEN_WIDTH = 640;
const int SCREEN_HEIGHT = 480;

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

	//Create window
	w->mWindow = SDL_CreateWindow(
				"SDL Tutorial",
				SDL_WINDOWPOS_UNDEFINED,
				SDL_WINDOWPOS_UNDEFINED,
				SCREEN_WIDTH,
				SCREEN_HEIGHT,
				SDL_WINDOW_SHOWN
				| SDL_WINDOW_RESIZABLE );
	if(w->mWindow == NULL) {
		printf( "Window could not be created! SDL Error: %s\n",
				SDL_GetError() );
		return -1;
	}
	
	w->mMouseFocus = 1;
	w->mKeyboardFocus = 1;
	w->mWidth = SCREEN_WIDTH;
	w->mHeight = SCREEN_HEIGHT;

	//Create renderer for window
	w->mRenderer = SDL_CreateRenderer(
				w->mWindow,
				-1,
				SDL_RENDERER_ACCELERATED |
				SDL_RENDERER_PRESENTVSYNC);
	if(w->mRenderer == NULL) {
		printf("Renderer could not be created! SDL Error: %s\n",
				SDL_GetError());
		SDL_DestroyWindow(w->mWindow);
		w->mWindow = NULL;
		return -1;
	}

	//Initialize renderer color
	SDL_SetRenderDrawColor(w->mRenderer, 0xFF, 0xFF, 0xFF, 0xFF);

	//Grab window identifier
	w->mWindowID = SDL_GetWindowID(w->mWindow);

	//Flag as opened
	w->mShown = 1;

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
			//Window appeared
			case SDL_WINDOWEVENT_SHOWN:
			w->mShown = 1;
			break;

			//Window disappeared
			case SDL_WINDOWEVENT_HIDDEN:
			w->mShown = 0;
			break;

			//Get new dimensions and repaint
			case SDL_WINDOWEVENT_SIZE_CHANGED:
			w->mWidth = e->window.data1;
			w->mHeight = e->window.data2;
			SDL_RenderPresent(w->mRenderer);
			break;

			//Repaint on expose
			case SDL_WINDOWEVENT_EXPOSED:
			SDL_RenderPresent(w->mRenderer);
			break;

			//Mouse enter
			case SDL_WINDOWEVENT_ENTER:
			w->mMouseFocus = 1;
			updateCaption = 1;
			break;
			
			//Mouse exit
			case SDL_WINDOWEVENT_LEAVE:
			w->mMouseFocus = 0;
			updateCaption = 1;
			break;

			//Keyboard focus gained
			case SDL_WINDOWEVENT_FOCUS_GAINED:
			w->mKeyboardFocus = 1;
			updateCaption = 1;
			break;
			
			//Keyboard focus lost
			case SDL_WINDOWEVENT_FOCUS_LOST:
			w->mKeyboardFocus = 0;
			updateCaption = 1;
			break;

			//Window minimized
			case SDL_WINDOWEVENT_MINIMIZED:
			w->mMinimized = 1;
			break;

			//Window maxized
			case SDL_WINDOWEVENT_MAXIMIZED:
			w->mMinimized = 0;
			break;
			
			//Window restored
			case SDL_WINDOWEVENT_RESTORED:
			w->mMinimized = 0;
			break;

			//Hide on close
			case SDL_WINDOWEVENT_CLOSE:
			SDL_HideWindow(w->mWindow);
			break;
		}

		if(updateCaption) {
			sprintf(caption, "%s%u%s%s%s%s",
				"SDL Tutorial - ID: ",
				w->mWindowID,
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
	//Restore window if needed
	if(w->mShown == 0)
		SDL_ShowWindow(w->mWindow);

	//Move window forward
	SDL_RaiseWindow(w->mWindow);
}

void LWindow_render(LWindow *w)
{
	if(w->mMinimized == 0) {	
		//Clear screen
		SDL_SetRenderDrawColor(w->mRenderer, 0xFF, 0xFF, 0xFF, 0xFF);
		SDL_RenderClear(w->mRenderer);

		//Update screen
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

int LWindow_getWidth(LWindow *w)
{
	return w->mWidth;
}

int LWindow_getHeight(LWindow *w)
{
	return w->mHeight;
}

short LWindow_hasMouseFocus(LWindow *w)
{
	return w->mMouseFocus;
}

short LWindow_hasKeyboardFocus(LWindow *w)
{
	return w->mKeyboardFocus;
}

short LWindow_isMinimized(LWindow *w)
{
	return w->mMinimized;
}

short LWindow_isShown(LWindow *w)
{
	return w->mShown;
}

short init()
{
	//Initialize SDL
	if(SDL_Init(SDL_INIT_VIDEO) < 0) {
		printf("SDL could not initialize! SDL Error: %s\n",
				SDL_GetError());
		return -1;
	}

	//Set texture filtering to linear
	if(SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "1") == 0)
		printf("Warning: Linear texture filtering not enabled!");

	//Create window
	if(LWindow_init(&gWindows[0])) {
		printf("Window 0 could not be created!\n");
		return -1;
	}

	return 0;
}

void close_all()
{
	int i;
	//Destroy windows
	for(i = 0; i < TOTAL_WINDOWS; ++i)
		LWindow_free(&gWindows[i]);

	//Quit SDL subsystems
	SDL_Quit();
}

int main(int argc, char* args[])
{
	int i;
	short allWindowsClosed;

	//Start up SDL and create window
	if(init()) {
		printf("Failed to initialize!\n" );
		goto equit;
	}

	//Initialize the rest of the windows
	for(i = 1; i < TOTAL_WINDOWS; ++i)
		LWindow_init(&gWindows[i]);

	SDL_Event e;

	while(1)
	{
		while(SDL_PollEvent(&e) != 0) {
			if(e.type == SDL_QUIT)
				goto equit;

			//Handle window events
			for(i = 0; i < TOTAL_WINDOWS; ++i)
				LWindow_handleEvent(&gWindows[i], &e);

			//Pull up window
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

		//Update all windows
		for(i = 0; i < TOTAL_WINDOWS; ++i)
			LWindow_render(&gWindows[i]);
			
		//Check all windows
		allWindowsClosed = 1;
		for(i = 0; i < TOTAL_WINDOWS; ++i)
			if(LWindow_isShown(&gWindows[i])) {
				allWindowsClosed = 0;
				break;
			}

		//Application closed all windows
		if(allWindowsClosed)
			goto equit;
	}
equit:
	//Free resources and close SDL
	close_all();

	return 0;
}
