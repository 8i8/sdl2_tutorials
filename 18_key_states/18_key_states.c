/*
 * Key States
 *
 * As we saw in the mouse input tutorial, there are ways to get the state of
 * the input devices (mouse, keyboard, etc) other than using events. In this
 * tutorial, we'll be remaking the keyboard input tutorial using key states
 * instead of events.
 */
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>

#define SCREEN_WIDTH	640
#define SCREEN_HEIGHT	480

typedef struct {
	SDL_Texture *mTexture;
	int mWidth;
	int mHeight;
} LTexture;

SDL_Window* gWindow = NULL;
SDL_Renderer* gRenderer = NULL;

LTexture gPressTexture;
LTexture gUpTexture;
LTexture gDownTexture;
LTexture gLeftTexture;
LTexture gRightTexture;

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
		SDL_Log("%s(), IMG_Load failed. %s", __func__, IMG_GetError());

		return -1;
	}

	SDL_SetColorKey(
			loadedSurface,
			SDL_TRUE,
			SDL_MapRGB(loadedSurface->format, 0, 0xFF, 0xFF));

	newTexture = SDL_CreateTextureFromSurface(gRenderer, loadedSurface);
	if(newTexture == NULL) {
		SDL_Log("%s(), SDL_CreateTextureFromSurface failed. %s", __func__, SDL_GetError());

		return -1;
	}

	lt->mWidth = loadedSurface->w;
	lt->mHeight = loadedSurface->h;

	SDL_FreeSurface(loadedSurface);

	lt->mTexture = newTexture;

	return 0;
}

short LTexture_render(
			LTexture *lt,
			int x,
			int y,
			SDL_Rect* clip,
			double angle,
			SDL_Point* center,
			SDL_RendererFlip flip)
{
	SDL_Rect renderQuad = { x, y, lt->mWidth, lt->mHeight };

	if( clip != NULL ) {
		renderQuad.w = clip->w;
		renderQuad.h = clip->h;
	}

	return SDL_RenderCopyEx(gRenderer, lt->mTexture, clip,
				&renderQuad, angle, center, flip);
}

short loadMedia(void)
{
	if(LTexture_loadFromFile(&gPressTexture, "press.png"))
		return -1;
	
	if(LTexture_loadFromFile(&gUpTexture, "up.png"))
		return -1;

	if(LTexture_loadFromFile(&gDownTexture, "down.png"))
		return -1;

	if(LTexture_loadFromFile(&gLeftTexture, "left.png"))
		return -1;

	if(LTexture_loadFromFile(&gRightTexture, "right.png"))
		return -1;

	return 0;
}

void close_all(void)
{
	free_texture(&gPressTexture);
	free_texture(&gUpTexture);
	free_texture(&gDownTexture);
	free_texture(&gLeftTexture);
	free_texture(&gRightTexture);

	SDL_DestroyRenderer(gRenderer);
	SDL_DestroyWindow(gWindow);
	gWindow = NULL;
	gRenderer = NULL;

	IMG_Quit();
	SDL_Quit();
}

/*
 * Right before we enter the main loop, we declare a texture pointer to keep
 * track of which texture we're rendering to the screen.
 */
/*
 * As you can see, we aren't checking for key events in the event loop. All our
 * keyboard input is going to be handled with key states.
 *
 * One important thing to know about how SDL handles key states is that you
 * still need an event loop running. SDL's internal keystates are updated every
 * time SDL_PollEvent is called, so make sure you polled all events on queue
 * before checking key states.
 */
/*
 * Here we set our texture that's going to be rendered. First we get a pointer
 * to the array of key states using SDL_GetKeyboardState. The state of all the
 * keys are ordered by SDL_Scancode. Scan codes are like the SDL_Keycode
 * values, only scan codes are designed to work with international keyboards.
 * Depending on the keyboard layout, different letters might be in different
 * places. Scan codes go off default physical position of the keys as opposed
 * to where they might be on a specific keyboard.
 *
 * All you have to do check if a key is down is to check its state in the key
 * state array. As you can see in the above code, if the key is down we set the
 * current texture to the corresponding texture. If none of the keys are down,
 * we set the default texture.
 */
/*
 * Finally here we render the current texture to the screen.
 */
int main(int argc, char* argv[])
{
	SDL_Event e;

	if(init())
		goto equit;

	if(loadMedia())
		goto equit;
	LTexture* currentTexture = NULL;
	const Uint8* currentKeyStates = SDL_GetKeyboardState(NULL);

	while(1)
	{
		while(SDL_PollEvent(&e) != 0)
			if(e.type == SDL_QUIT)
				goto equit;

		if(currentKeyStates[SDL_SCANCODE_UP]) {
			currentTexture = &gUpTexture;
		}
		else if(currentKeyStates[SDL_SCANCODE_DOWN]) {
			currentTexture = &gDownTexture;
		}
		else if(currentKeyStates[SDL_SCANCODE_LEFT]) {
			currentTexture = &gLeftTexture;
		}
		else if(currentKeyStates[SDL_SCANCODE_RIGHT]) {
			currentTexture = &gRightTexture;
		}
		else
			currentTexture = &gPressTexture;

		SDL_SetRenderDrawColor(gRenderer, 0xFF, 0xFF, 0xFF, 0xFF);
		SDL_RenderClear(gRenderer);

		LTexture_render(currentTexture, 0, 0, NULL, 0.0, NULL,
						SDL_FLIP_NONE);

		SDL_RenderPresent(gRenderer);
	}
equit:
	close_all();

	return 0;
}

