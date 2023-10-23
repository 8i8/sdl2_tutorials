/*
 * Key Presses
 *
 * Xing out the window is just one of the events SDL is capable of handling.
 * Another type of input used heavily in games is the keyboard. In this
 * tutorial we're going to make different images appear depending on which key
 * you press.
 */
#include <SDL2/SDL.h>

#define SCREEN_WIDTH	640
#define SCREEN_HEIGHT	480

/*
 * Near the top of the source code we declare an enumeration of the different
 * surfaces we have. Enumerations are a shorthand way to do symbolic constants
 * instead of having to do const int KEY_PRESS_SURFACE_DEFAULT = 0; const int
 * KEY_PRESS_SURFACE_UP = 1; const int KEY_PRESS_SURFACE_DOWN = 2; and such.
 * They default to start counting at 0 and go up by one for each enumeration
 * you declare. This means KEY_PRESS_SURFACE_DEFAULT is 0, KEY_PRESS_SURFACE_UP
 * is 1, KEY_PRESS_SURFACE_DOWN is 2, KEY_PRESS_SURFACE_LEFT is 3,
 * KEY_PRESS_SURFACE_RIGHT is 4, and KEY_PRESS_SURFACE_TOTAL is 5, It's
 * possible to give them explicit integer values, but we won't be covering that
 * here. A quick Google search on enumerations should cover that.
 *
 * One bad habit beginning programmers have is using abritary numbers instead
 * of symbolic constants. For example they'll have 1 mean main menu, 2 mean
 * options, etc which is fine for small programs. When you're dealing with
 * thousands of lines of code (which video games usually do), having a line
 * that says "if( option == 1 )" will produce much more headaches than using
 * "if( option == MAIN_MENU )".
 */
enum KeyPressSurfaces
{
	KEY_PRESS_SURFACE_DEFAULT,
	KEY_PRESS_SURFACE_UP,
	KEY_PRESS_SURFACE_DOWN,
	KEY_PRESS_SURFACE_LEFT,
	KEY_PRESS_SURFACE_RIGHT,
	KEY_PRESS_SURFACE_TOTAL
};

/*
 * Along with our usual function prototypes, we have a new function called
 * loadSurface. There's a general rule that if you're copy/pasting code, you're
 * doing something wrong. Rather than copy/paste loading code every time, we're
 * going to use a function to handle that.
 *
 * What's important to this specific program is that we have an array of
 * pointers to SDL surfaces called gKeyPressSurfaces to contain all the images
 * we'll be using. Depending on which key the user presses, we'll set
 * gCurrentSurface (which is the image that will be blitted to the screen) to
 * one of these surfaces.
 */
SDL_Surface* loadSurface(char *path);
SDL_Window* gWindow = NULL;
SDL_Surface* gScreenSurface = NULL;
SDL_Surface* gCurrentSurface = NULL;
SDL_Surface* gKeyPressSurfaces[KEY_PRESS_SURFACE_TOTAL];

short init(void)
{
	if(SDL_Init(SDL_INIT_VIDEO) < 0) {
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

	gScreenSurface = SDL_GetWindowSurface(gWindow);

	return 0;
}

/*
 * Here in the loadMedia function we load all of the images we're going to
 * render to the screen.
 *
 * And since I get this question a lot by new C++ programmers, no this function
 * does not leak memory. It does allocate memory to load a new SDL surface and
 * return it without freeing the allocated memory, but what would be the point
 * of allocating the surface and immediately deallocating it? What this
 * function does is load the surface and return the newly loaded surface so
 * what ever called this function can deallocate it after it's done using it.
 * In this program, the loaded surface is deallocated in the close function.
 */
short loadMedia(void)
{
	gKeyPressSurfaces[KEY_PRESS_SURFACE_DEFAULT] = SDL_LoadBMP("press.bmp");
	if(gKeyPressSurfaces[KEY_PRESS_SURFACE_DEFAULT] == NULL) {
		SDL_Log("%s(), SDL_LoadBMP faild. %s", __func__, SDL_GetError());
		return -1;
	}

	gKeyPressSurfaces[KEY_PRESS_SURFACE_UP] = SDL_LoadBMP("up.bmp");
	if(gKeyPressSurfaces[KEY_PRESS_SURFACE_UP] == NULL) {
		SDL_Log("%s(), SDL_LoadBMP faild. %s", __func__, SDL_GetError());
		return -1;
	}

	gKeyPressSurfaces[KEY_PRESS_SURFACE_DOWN] = SDL_LoadBMP("down.bmp");
	if(gKeyPressSurfaces[KEY_PRESS_SURFACE_DOWN] == NULL) {
		SDL_Log("%s(), SDL_LoadBMP faild. %s", __func__, SDL_GetError());
		return -1;
	}

	gKeyPressSurfaces[KEY_PRESS_SURFACE_LEFT] = SDL_LoadBMP("left.bmp");
	if(gKeyPressSurfaces[KEY_PRESS_SURFACE_LEFT] == NULL) {
		SDL_Log("%s(), SDL_LoadBMP faild. %s", __func__, SDL_GetError());
		return -1;
	}

	gKeyPressSurfaces[KEY_PRESS_SURFACE_RIGHT] = SDL_LoadBMP("right.bmp");
	if(gKeyPressSurfaces[KEY_PRESS_SURFACE_RIGHT] == NULL) {
		SDL_Log("%s(), SDL_LoadBMP faild. %s", __func__, SDL_GetError());
		return -1;
	}

	return 0;
}

void close_all(void)
{
	int i;

	for (i = 0; i < KEY_PRESS_SURFACE_TOTAL; ++i) {
		SDL_FreeSurface(gKeyPressSurfaces[i]);
		gKeyPressSurfaces[i] = NULL;
	}

	SDL_DestroyWindow(gWindow);
	gWindow = NULL;

	SDL_Quit();
}

/*
 * Inside of the SDL Event is an SDL Keyboard event which contains the
 * information for the key event. Inside of that is a SDL Keysym which contains
 * the information about the key that was pressed. That Keysym contains the SDL
 * Keycode which identifies the key that was pressed.
 * 
 * As you can see, what this code does is set the surfaces based on which key
 * was pressed. Look in the SDL documentation if you want to see what the other
 * keycodes are for other keys.
 */
int get_event(SDL_Event e)
{
	if(e.type == SDL_QUIT)
		return -1;
	if(e.type == SDL_KEYDOWN)
	{
		switch(e.key.keysym.sym)
		{
			case SDLK_UP:
			gCurrentSurface = gKeyPressSurfaces[KEY_PRESS_SURFACE_UP];
			break;

			case SDLK_DOWN:
			gCurrentSurface = gKeyPressSurfaces[KEY_PRESS_SURFACE_DOWN];
			break;

			case SDLK_LEFT:
			gCurrentSurface = gKeyPressSurfaces[KEY_PRESS_SURFACE_LEFT];
			break;

			case SDLK_RIGHT:
			gCurrentSurface = gKeyPressSurfaces[KEY_PRESS_SURFACE_RIGHT];
			break;

			default:
			gCurrentSurface = gKeyPressSurfaces[KEY_PRESS_SURFACE_DEFAULT];
			break;
		}
	}
	return 0;
}

/*
 * In the main function before entering the main loop, we set the default
 * surface to display. 
 */
int main(int argc, char* args[])
{
	SDL_Event e;

	if(init())
		goto equit;
	if(loadMedia())
		goto equit;

	gCurrentSurface = gKeyPressSurfaces[KEY_PRESS_SURFACE_DEFAULT];
/*
 * Here is our event loop. As you can see handle closing the window as we did
 * in the previous tutorial, then we handle an SDL_KEYDOWN event from inside of
 * the get_event() function, that is described above; This event happens when
 * ever you press a key on the keyboard.
 */
	while(1)
	{
		while(SDL_PollEvent(&e)!= 0)
			if(get_event(e))
				goto equit;
/*
 * After the keys have been handled and the surface has been set we blit the
 * selected surface to the screen.
 */
		SDL_BlitSurface(gCurrentSurface, NULL, gScreenSurface, NULL);
		SDL_UpdateWindowSurface(gWindow);
		SDL_Delay(60);
	}
equit:
	close_all();

	return 0;
}

