/*
 * Now that you've already got a window open, let's put an image on it.
 *
 * Note: From now on the tutorials will only cover key parts of source code.
 * For the full program, you will have to download the full source code. 
 */
#include <SDL2/SDL.h>

#define SCREEN_WIDTH	640
#define SCREEN_HEIGHT	480

SDL_Window* wWindow = NULL;
SDL_Surface* sMainSurface = NULL;      /* SDL_Surface uses cpu rendering */
SDL_Surface* sHelloWorld = NULL;

short init(void)
{
	if(SDL_Init(SDL_INIT_VIDEO) < 0) {
		SDL_Log("%s(), SDL_Init failed. %s", __func__, SDL_GetError());
		return -1;
	}

	wWindow = SDL_CreateWindow(
					"SDL Tutorial",
					SDL_WINDOWPOS_UNDEFINED,
					SDL_WINDOWPOS_UNDEFINED,
					SCREEN_WIDTH, SCREEN_HEIGHT,
					SDL_WINDOW_SHOWN);
	if(wWindow == NULL) {
		SDL_Log("%s(), SDL_CreateWindow failed. %s", __func__, SDL_GetError());
		return -1;
	}
	
	sMainSurface = SDL_GetWindowSurface(wWindow);

	return 0;
}

short loadMedia(void)
{
	sHelloWorld = SDL_LoadBMP("hello_world.bmp");

	if(sHelloWorld == NULL) {
		SDL_Log("%s(), SDL_LoadBMP failed. %s", __func__, SDL_GetError());
		return -1;
	}

	return 0;
}

void close_all(void)
{
	SDL_FreeSurface(sHelloWorld);
	sHelloWorld = NULL;

	SDL_DestroyWindow(wWindow);
	wWindow = NULL;

	SDL_Quit();
}

int main(int argc, char* args[])
{
	if(init())
		goto equit;

	if(loadMedia())
		goto equit;

	SDL_Event e;
	while(1)
	{
		while(SDL_PollEvent(&e))
			if(e.type == SDL_QUIT)
				goto equit;

		sMainSurface = SDL_GetWindowSurface(wWindow);
		SDL_BlitSurface(sHelloWorld, NULL, sMainSurface, NULL);
		SDL_UpdateWindowSurface(wWindow);
		SDL_Delay(60);
	}
equit:
	close_all();

	return 0;
}

/*
 * In the first tutorial, we put everything in the main function. Since it was
 * a small program we can get away with that, but in real programs (like video
 * games) you want to have your code as modular as possible. This means you
 * want your code to be in neat chunks that are each easy to debug and reuse.
 *
 * Here it means we have functions to handle initialization, loading media, and
 * closing down the SDL application. We declare these near the top of our
 * source file.
 */
/*
 * Here's a new data type called an SDL Surface. An SDL surface is just an
 * image data type that contains the pixels of an image along with all data
 * needed to render it. SDL surfaces use software rendering which means it uses
 * the CPU to render. It is possible to render hardware images but it's a bit
 * more difficult so we're going to learn it the easy way first. In future
 * tutorials we'll cover how to render GPU accelerated images.
 *
 * The images we're going to be dealing with here are the screen image (what
 * you see inside of the window) and the image we'll be loading from a file.
 *
 * Notice that these are pointers to SDL surfaces. The reason is that 1) we'll
 * be dynamically allocating memory to load images and 2) it's better to
 * reference an image by memory location. Imagine you had a game with a brick
 * wall that consisted of the same brick image being rendered multiple times
 * (like Super Mario Bros). It's wasteful to have dozens of copies of the image
 * in memory when you can have one copy of the image and render it over and
 * over again.
 *
 * Also, always remember to initialize your pointers. We set them to NULL
 * immediately when declaring them. 
 */

/*
 * As you can see here, we've taken the SDL initialization and the window
 * creation code and put it in its own function. What's new is that there's a
 * call to SDL_GetWindowSurface.
 *
 * We want to show images inside of the window and in order to do that we need
 * to get the image inside of the window. So we call SDL_GetWindowSurface to
 * grab the surface contained by the window.
 */

/*
 * In the load media function we load our image using SDL_LoadBMP. SDL_LoadBMP
 * takes in the path of a bmp file and returns the loaded surface. If the
 * function returns NULL, that means it failed so we print to the console an
 * error using SDL_Log.
 *
 * An important thing to note is that this piece of code assumes you have a
 * directory called "02_getting_an_image_on_the_screen" that contains an image
 * named "hello_world.bmp" in your working directory. The working directory is
 * where your application thinks it is operating. Typically, your working
 * directory is the directory where your executable is at but some programs
 * like Visual Studio change the working directory to where the vcxproj file is
 * located. So if your application can't find the image, make sure it is in the
 * right place. 
 */

/*
 * In our clean up code, we destroy the window and quit SDL like before but we
 * also have to take care of the surface we loaded. We do this by freeing it
 * with SDL_FreeSurface. Don't worry about the screen surface,
 * SDL_DestroyWindow will take care of it.
 *
 * Make sure to get into the habit of having your pointers point to NULL when
 * they're not pointing to anything.
 */

/*
 * In our main function we initialize SDL and load the image. If that succeeded
 * we blit the loaded surface onto the screen surface using SDL_BlitSurface.
 * 
 * What blitting does is take a source surface and stamps a copy of it onto the
 * destination surface. The first argument of SDL_BlitSurface is the source
 * image. The third argument is the destination. We'll worry about the 2nd and
 * 4th arguments in future tutorials.
 * 
 * Now if this was the only code for drawing we had, we still wouldn't see the
 * image we loaded on the screen. There's one more step. 
 */

/*
 * After drawing everything on the screen that we want to show for this frame
 * we have to update the screen using SDL_UpdateWindowSurface. See when you
 * draw to the screen, you are not typically drawing to the image on the screen
 * you see. By default, most rendering systems out there are double buffered.
 * These two buffers are the front and back buffer.
 * 
 * When you make draw calls like SDL_BlitSurface, you render to the back
 * buffer. What you see on the screen is the front buffer. The reason we do
 * this is because most frames require drawing multiple objects to the screen.
 * If we only had a front buffer, we would be able to see the frame as things
 * are being drawn to it which means we would see unfinished frames. So what we
 * do is draw everything to the back buffer first and once we're done we swap
 * the back and front buffer so now the user can see the finished frame.
 * 
 * This also means that you don't call SDL_UpdateWindowSurface after every
 * blit, only after all the blits for the current frame are done.
 */

/*
 * Now that we've rendered everything to the window, we delay for two seconds
 * so the window doesn't just disappear. After the wait is done, we close out
 * our program.
 */
