/*
 * Event Driven Programming
 * 
 * Besides just putting images on the screen, games require that you handle
 * input from the user. You can do that with SDL using the event handling
 * system.
 */
#include <SDL2/SDL.h>

#define SCREEN_WIDTH	640
#define SCREEN_HEIGHT	480

SDL_Window* gWindow = NULL;
SDL_Surface* gScreenSurface = NULL;
SDL_Surface* gXOut = NULL;

int init(void)
{
	if(SDL_Init(SDL_INIT_VIDEO) < 0) {
		SDL_Log("%s(), SDL_Init failed. %s", __func__, SDL_GetError());
		return -1;
	}

	if((gWindow = SDL_CreateWindow(
					"SDL Tutorial",
					SDL_WINDOWPOS_UNDEFINED,
					SDL_WINDOWPOS_UNDEFINED,
					SCREEN_WIDTH,
					SCREEN_HEIGHT,
					SDL_WINDOW_SHOWN
					)) == NULL)
	{
		SDL_Log("%s(), SDL_CreateWindow failed. %s", __func__, SDL_GetError());
		return -1;
	}

	gScreenSurface = SDL_GetWindowSurface(gWindow);

	return 0;
}

int loadMedia(void)
{
	if((gXOut = SDL_LoadBMP("x.bmp")) == NULL) {
		SDL_Log("%s(), SDL_LoadBMP failed. %s", __func__, SDL_GetError());
		return -1;
	}

	return 0;
}

void close_all(void)
{
	SDL_FreeSurface(gXOut);
	gXOut = NULL;
	SDL_DestroyWindow(gWindow);
	gWindow = NULL;
	SDL_Quit();
}

int main(int argc, char* argv[])
{
	SDL_Event e;

	if(init())
		goto equit;

	if(loadMedia())
		goto equit;

	while(1)
	{
		while(SDL_PollEvent(&e)!= 0)
			if(e.type == SDL_QUIT)
				goto equit;

		SDL_BlitSurface(gXOut, NULL, gScreenSurface, NULL);
		SDL_UpdateWindowSurface(gWindow);
		SDL_Delay(60);
	}
equit:
	close_all();

	return 0;
}

/*
 * In our code after SDL is initialized and the media is loaded (as mentioned
 * in the previous tutorial).
 *
 * We also declare an SDL_Event union. A SDL event is some thing like a key
 * press, mouse motion, joy button press, etc. In this application we're going
 * to look for quit events to end the application.
 */

/*
 * In our code after SDL is initialized and the media is loaded, as mentioned
 * in ../01_hello_SDL/01_hello_SDL.c, we create a goto event and an infinite
 * while loop that allows the while loop to be broke out of.
 *
 * We also declare an SDL_Event union. A SDL event is some thing like a key
 * press, mouse motion, joy button press, etc. In this application we're going
 * to look for quit events to end the application.
 */

/*
 * In the previous tutorials, we had the program wait for a few seconds before
 * closing. In this application we're having the application wait until the
 * user quits before closing.
 *
 * So we'll have the application loop while the user has not quit. This loop
 * that keeps running while the application is active is called the main loop,
 * which is sometimes called the game loop. It is the core of any game
 * application. 
 *
 * At the top of our main loop we have our event loop. What this does is keep
 * processing the event queue until it is empty.
 *
 * When you press a key, move the mouse, or touch a touch screen you put events
 * onto the event queue.
 *
 * The event queue will then store them in the order the events occured waiting
 * for you to process them. When you want to find out what events occured so
 * you can process them, you poll the event queue to get the most recent event
 * by calling SDL_PollEvent. What SDL_PollEvent does is take the most recent
 * event from the event queue and puts the data from the event into the
 * SDL_Event we passed into the function. 
 *
 * SDL_PollEvent will keep taking events off the queue until it is empty. When
 * the queue is empty, SDL_PollEvent will return 0. So what this piece of code
 * does is keep polling events off the event queue until it's empty. If an
 * event from the event queue is an SDL_QUIT event (which is the event when the
 * user Xs out the window), we set the quit flag to true so we can exit the
 * application.
 */

/*
 * After we're done processing the events for our frame, we draw to the screen
 * and update it (as discussed in
 * ../02_getting_an_image_on_the_screen/02_getting_an_image_on_the_screen.c).
 * If the quit flag was set to true, the application will exit at the end of
 * the loop. If it is still false it will keep going until the user Xs out the
 * window.
 */
