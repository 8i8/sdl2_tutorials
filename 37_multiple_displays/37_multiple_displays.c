/*
 * Multiple Displays
 *
 * Another neat new feature with SDL 2 is the ability to handle multiple
 * displays. Here we'll be making our window jump from display to display.
 */
#include <SDL2/SDL.h>
#include <stdio.h>

#define SCREEN_WIDTH	640
#define SCREEN_HEIGHT	480

/*
 * Here is our window from previous tutorials with a window display ID to keep
 * track of which display the window is on.
 */
typedef struct {
	SDL_Window* mWindow;
	SDL_Renderer* mRenderer;
	unsigned mWindowID;
	unsigned mWindowDisplayID;

	int mWidth;
	int mHeight;

	short mMouseFocus;
	short mKeyboardFocus;
	short mFullScreen;
	short mMinimized;
	short mShown;
} LWindow;

/*
 * Our displays all have a integer ID and a rectangle associated with them so
 * we know the position and dimensions of each display on our desktop.
 */
LWindow gWindow;
unsigned gTotalDisplays = 0;
SDL_Rect* gDisplayBounds = NULL; 

void LWindow_new(LWindow *w)
{
	w->mWindow = NULL;
	w->mRenderer = NULL;

	w->mMouseFocus = 0;
	w->mKeyboardFocus = 0;
	w->mFullScreen = 0;
	w->mShown = 0;
	w->mWindowID = -1;
	w->mWindowDisplayID = -1;

	w->mWidth = 0;
	w->mHeight = 0;
}

/*
 * Our window creation code is pretty much the same as before only now we made
 * a call to SDL_GetWindowDisplayIndex so we know which display the window was
 * created on.
 */
short LWindow_init(LWindow *w)
{
	w->mWindow = SDL_CreateWindow(
					"SDL Tutorial",
					SDL_WINDOWPOS_UNDEFINED,
					SDL_WINDOWPOS_UNDEFINED,
					SCREEN_WIDTH,
					SCREEN_HEIGHT,
					SDL_WINDOW_SHOWN
					| SDL_WINDOW_RESIZABLE);
	if(w->mWindow == NULL) {
		SDL_Log("%s(), SDL_CreateWindow failed. %s", __func__,
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
					SDL_RENDERER_ACCELERATED
					| SDL_RENDERER_PRESENTVSYNC);
	if(w->mRenderer == NULL) {
		printf("Renderer could not be created! SDL Error: %s\n",
				SDL_GetError());
		SDL_DestroyWindow(w->mWindow);
		w->mWindow = NULL;
		return -1;
	}

	SDL_SetRenderDrawColor(w->mRenderer, 0xFF, 0xFF, 0xFF, 0xFF);

	w->mWindowID = SDL_GetWindowID(w->mWindow);
	w->mWindowDisplayID = SDL_GetWindowDisplayIndex(w->mWindow);

	w->mShown = 1;

	return 0;
}

/*
 * Here in our window's event handler we handle a SDL_WINDOWEVENT_MOVED event
 * so we can update the display the window is on using
 * SDL_GetWindowDisplayIndex. 
 *
 * When we press up or down we change the display index to move to the next
 * display.
 *
 * If we need to move to the next display, we first make sure the display is a
 * valid index by bounding it. We then update the position of the window with
 * SDL_SetWindowPosition. This call here will center the window in the next
 * display.
 */
void LWindow_handleEvent(LWindow *w, SDL_Event *e)
{
	short updateCaption = 0;
	char caption[255];

	if(e->type == SDL_WINDOWEVENT && e->window.windowID == w->mWindowID)
	{
		switch(e->window.event)
		{
			case SDL_WINDOWEVENT_MOVED:
			w->mWindowDisplayID = SDL_GetWindowDisplayIndex(w->mWindow);
			updateCaption = 1;
			goto update;

			case SDL_WINDOWEVENT_SHOWN:
			w->mShown = 1;
			goto update;

			case SDL_WINDOWEVENT_HIDDEN:
			w->mShown = 0;
			goto update;

			case SDL_WINDOWEVENT_SIZE_CHANGED:
			w->mWidth = e->window.data1;
			w->mHeight = e->window.data2;
			SDL_RenderPresent(w->mRenderer);
			goto update;

			case SDL_WINDOWEVENT_EXPOSED:
			SDL_RenderPresent(w->mRenderer);
			goto update;

			case SDL_WINDOWEVENT_ENTER:
			w->mMouseFocus = 1;
			updateCaption = 1;
			goto update;
			
			case SDL_WINDOWEVENT_LEAVE:
			w->mMouseFocus = 0;
			updateCaption = 1;
			goto update;

			case SDL_WINDOWEVENT_FOCUS_GAINED:
			w->mKeyboardFocus = 1;
			updateCaption = 1;
			goto update;
			
			case SDL_WINDOWEVENT_FOCUS_LOST:
			w->mKeyboardFocus = 0;
			updateCaption = 1;
			goto update;

			case SDL_WINDOWEVENT_MINIMIZED:
			w->mMinimized = 1;
			goto update;

			case SDL_WINDOWEVENT_MAXIMIZED:
			w->mMinimized = 0;
			goto update;
			
			case SDL_WINDOWEVENT_RESTORED:
			w->mMinimized = 0;
			goto update;

			case SDL_WINDOWEVENT_CLOSE:
			SDL_HideWindow(w->mWindow);
			goto update;
		}
	}

	if(e->type == SDL_KEYDOWN)
	{
		short switchDisplay = 0;

		switch(e->key.keysym.sym)
		{
			case SDLK_UP:
			++w->mWindowDisplayID;
			switchDisplay = 1;
			goto update;

			case SDLK_DOWN:
			--w->mWindowDisplayID;
			switchDisplay = 1;
			goto update;
		}

		if(switchDisplay == 0)
			goto update;

		if(w->mWindowDisplayID != 0)
			w->mWindowDisplayID = gTotalDisplays - 1;
		else if(w->mWindowDisplayID >= gTotalDisplays)
			w->mWindowDisplayID = 0;

		SDL_SetWindowPosition(
				w->mWindow,
				gDisplayBounds[w->mWindowDisplayID].x
				+ (gDisplayBounds[w->mWindowDisplayID].w - w->mWidth) / 2,
				gDisplayBounds[w->mWindowDisplayID].y
				+ (gDisplayBounds[w->mWindowDisplayID].h - w->mHeight) / 2);
		updateCaption = 1;
	}
update:
	if(updateCaption) {
		sprintf(caption, "%s%u%s%u%s%s%s%s",
			"SDL Tutorial - ID: ",
			w->mWindowID-1,
			" Display: ",
			w->mWindowDisplayID,
			" MouseFocus:",
			((w->mMouseFocus) ? "On" : "Off"),
			" KeyboardFocus:",
			((w->mKeyboardFocus) ? "On" : "Off"));
		SDL_SetWindowTitle(w->mWindow, caption);
	}
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

/*
 * In our initialization function we find out how many displays are connect to
 * the computer using SDL_GetNumVideoDisplays. If there's only 1 display we
 * output a warning. 
 */
short init(void)
{
	if(SDL_Init(SDL_INIT_VIDEO) < 0) {
		SDL_Log("%s(), SDL_Init failed. %s", __func__,
				SDL_GetError());
		return -1;
	}

	if(SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "1") == 0)
		SDL_Log("Warning: Linear texture filtering not enabled.");

	gTotalDisplays = SDL_GetNumVideoDisplays();
	if(gTotalDisplays < 2)
		SDL_Log("%s(), Warning: Only one display connected.", __func__);
/*
 * Now that we know how many displays are connected, we allocate rectangles for
 * each of them and get the bounds for each one using SDL_GetDisplayBounds.
 * After this we initialize our window.
 */
	gDisplayBounds = malloc(sizeof(SDL_Rect[gTotalDisplays]));
	size_t i;
	for(i = 0; i < gTotalDisplays; ++i)
		SDL_GetDisplayBounds(i, &gDisplayBounds[i]);

	if(LWindow_init(&gWindow) < 0)
		return -1;

	return 0;
}

void close_all(void)
{
	LWindow_free(&gWindow);
	free(gDisplayBounds);

	SDL_Quit();
}

/*
 * Since our code is well encapsulated the main loop hasn't changed since all
 * the changes have happened under the hood.
 */
int main(int argc, char* args[])
{
	if(init())
		goto equit;

	SDL_Event e;

	while(1)
	{
		while(SDL_PollEvent(&e) != 0) {
			if(e.type == SDL_QUIT)
				goto equit;

			LWindow_handleEvent(&gWindow, &e);
		}

		LWindow_render(&gWindow);
	}
equit:
	close_all();

	return 0;
}

