/*This source code copyrighted by Lazy Foo' Productions (2004-2015)
and may not be redistributed without written permission.*/

//Using SDL, standard IO, strings, and string streams
#include <SDL2/SDL.h>
#include <stdio.h>

//Screen dimension constants
const int SCREEN_WIDTH = 640;
const int SCREEN_HEIGHT = 480;

//Total windows
const int TOTAL_WINDOWS = 3;

class LWindow
{
	public:
		//Intializes internals
		LWindow();

		//Creates window
		short init();

		//Handles window events
		void handleEvent(SDL_Event& e);

		//Focuses on window
		void focus();

		//Shows windows contents
		void render();

		//Deallocates internals
		void free();

		//Window dimensions
		int getWidth();
		int getHeight();

		//Window focii
		short hasMouseFocus();
		short hasKeyboardFocus();
		short isMinimized();
		short isShown();

	private:
		//Window data
		SDL_Window* mWindow;
		SDL_Renderer* mRenderer;
		unsigned mWindowID;

		//Window dimensions
		int mWidth;
		int mHeight;

		//Window focus
		short mMouseFocus;
		short mKeyboardFocus;
		short mFullScreen;
		short mMinimized;
		short mShown;
};

//Our custom windows
LWindow gWindows[TOTAL_WINDOWS];

LWindow::LWindow()
{
	//Initialize non-existant window
	mWindow = NULL;
	mRenderer = NULL;

	mMouseFocus = 0;
	mKeyboardFocus = 0;
	mFullScreen = 0;
	mShown = 0;
	mWindowID = -1;
	
	mWidth = 0;
	mHeight = 0;
}

short LWindow::init()
{
	//Create window
	mWindow = SDL_CreateWindow(
				"SDL Tutorial",
				SDL_WINDOWPOS_UNDEFINED,
				SDL_WINDOWPOS_UNDEFINED,
				SCREEN_WIDTH,
				SCREEN_HEIGHT,
				SDL_WINDOW_SHOWN
				| SDL_WINDOW_RESIZABLE );
	if(mWindow == NULL) {
		printf( "Window could not be created! SDL Error: %s\n",
				SDL_GetError() );
		return -1;
	}
	
	mMouseFocus = 1;
	mKeyboardFocus = 1;
	mWidth = SCREEN_WIDTH;
	mHeight = SCREEN_HEIGHT;

	//Create renderer for window
	mRenderer = SDL_CreateRenderer(
				mWindow,
				-1,
				SDL_RENDERER_ACCELERATED |
				SDL_RENDERER_PRESENTVSYNC);
	if(mRenderer == NULL) {
		printf( "Renderer could not be created! SDL Error: %s\n",
				SDL_GetError());
		SDL_DestroyWindow(mWindow);
		mWindow = NULL;
		return -1;
	}

	//Initialize renderer color
	SDL_SetRenderDrawColor(mRenderer, 0xFF, 0xFF, 0xFF, 0xFF);

	//Grab window identifier
	mWindowID = SDL_GetWindowID(mWindow);

	//Flag as opened
	mShown = 1;

	return 0;
}

void LWindow::handleEvent(SDL_Event& e)
{
	//If an event was detected for this window
	if(e.type == SDL_WINDOWEVENT && e.window.windowID == mWindowID)
	{
		//Caption update flag
		short updateCaption = 0;
		char caption[255];

		switch( e.window.event )
		{
			//Window appeared
			case SDL_WINDOWEVENT_SHOWN:
			mShown = 1;
			break;

			//Window disappeared
			case SDL_WINDOWEVENT_HIDDEN:
			mShown = 0;
			break;

			//Get new dimensions and repaint
			case SDL_WINDOWEVENT_SIZE_CHANGED:
			mWidth = e.window.data1;
			mHeight = e.window.data2;
			SDL_RenderPresent(mRenderer);
			break;

			//Repaint on expose
			case SDL_WINDOWEVENT_EXPOSED:
			SDL_RenderPresent(mRenderer);
			break;

			//Mouse enter
			case SDL_WINDOWEVENT_ENTER:
			mMouseFocus = 1;
			updateCaption = 1;
			break;
			
			//Mouse exit
			case SDL_WINDOWEVENT_LEAVE:
			mMouseFocus = 0;
			updateCaption = 1;
			break;

			//Keyboard focus gained
			case SDL_WINDOWEVENT_FOCUS_GAINED:
			mKeyboardFocus = 1;
			updateCaption = 1;
			break;
			
			//Keyboard focus lost
			case SDL_WINDOWEVENT_FOCUS_LOST:
			mKeyboardFocus = 0;
			updateCaption = 1;
			break;

			//Window minimized
			case SDL_WINDOWEVENT_MINIMIZED:
			mMinimized = 1;
			break;

			//Window maxized
			case SDL_WINDOWEVENT_MAXIMIZED:
			mMinimized = 0;
			break;
			
			//Window restored
			case SDL_WINDOWEVENT_RESTORED:
			mMinimized = 0;
			break;

			//Hide on close
			case SDL_WINDOWEVENT_CLOSE:
			SDL_HideWindow(mWindow);
			break;
		}

		//Update window caption with new data
		if(updateCaption) {
			sprintf(caption, "%s%u%s%s%s%s",
				"SDL Tutorial - ID: ",
				mWindowID,
				" MouseFocus:",
				((mMouseFocus) ? "On" : "Off"),
				" KeyboardFocus:",
				((mKeyboardFocus) ? "On" : "Off"));
			SDL_SetWindowTitle(mWindow, caption);
		}
	}
}

void LWindow::focus()
{
	//Restore window if needed
	if(mShown == 0)
		SDL_ShowWindow(mWindow);

	//Move window forward
	SDL_RaiseWindow(mWindow);
}

void LWindow::render()
{
	if(mMinimized == 0) {	
		//Clear screen
		SDL_SetRenderDrawColor(mRenderer, 0xFF, 0xFF, 0xFF, 0xFF);
		SDL_RenderClear(mRenderer);

		//Update screen
		SDL_RenderPresent(mRenderer);
	}
}

void LWindow::free()
{
	if(mWindow != NULL)
		SDL_DestroyWindow( mWindow );

	mMouseFocus = 0;
	mKeyboardFocus = 0;
	mWidth = 0;
	mHeight = 0;
}

int LWindow::getWidth()
{
	return mWidth;
}

int LWindow::getHeight()
{
	return mHeight;
}

short LWindow::hasMouseFocus()
{
	return mMouseFocus;
}

short LWindow::hasKeyboardFocus()
{
	return mKeyboardFocus;
}

short LWindow::isMinimized()
{
	return mMinimized;
}

short LWindow::isShown()
{
	return mShown;
}

short init()
{
	//Initialize SDL
	if(SDL_Init(SDL_INIT_VIDEO) < 0) {
		printf( "SDL could not initialize! SDL Error: %s\n",
				SDL_GetError());
		return -1;
	}

	//Set texture filtering to linear
	if(SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "1") == 0)
		printf("Warning: Linear texture filtering not enabled!");

	//Create window
	if(gWindows[0].init()) {
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
		gWindows[i].free();

	//Quit SDL subsystems
	SDL_Quit();
}

int main(int argc, char* args[])
{
	int i;

	//Start up SDL and create window
	if(init()) {
		printf( "Failed to initialize!\n" );
		goto equit;
	}

	//Initialize the rest of the windows
	for(i = 1; i < TOTAL_WINDOWS; ++i)
		gWindows[i].init();

	SDL_Event e;

	//While application is running
	while(1)
	{
		while(SDL_PollEvent(&e) != 0) {
			if(e.type == SDL_QUIT)
				goto equit;

			//Handle window events
			for(i = 0; i < TOTAL_WINDOWS; ++i )
				gWindows[ i ].handleEvent( e );

			//Pull up window
			if(e.type == SDL_KEYDOWN)
			{
				switch(e.key.keysym.sym)
				{
					case SDLK_1:
					gWindows[0].focus();
					break;

					case SDLK_2:
					gWindows[1].focus();
					break;
						
					case SDLK_3:
					gWindows[2].focus();
					break;

					case SDLK_q:
					goto equit;
					break;
				}
			}
		}

		//Update all windows
		for(i = 0; i < TOTAL_WINDOWS; ++i)
			gWindows[ i ].render();
			
		//Check all windows
		short allWindowsClosed = 1;
		for(i = 0; i < TOTAL_WINDOWS; ++i)
			if( gWindows[i].isShown()) {
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
