/*
 * Motion
 *
 * Now that we know how to render, handle input, and deal with time we can now
 * know everything we need move around things on the screen. Here will do a
 * basic dot moving around.
 */
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>

#define SCREEN_WIDTH	640
#define SCREEN_HEIGHT	480
#define DOT_WIDTH	20
#define DOT_HEIGHT	20
#define DOT_VEL		5
#define DOT_JOY_VEL	1
#define JOYSTICK_DEAD_ZONE	8000

typedef struct {
	SDL_Texture *mTexture;
	int mWidth;
	int mHeight;
} LTexture;

/*
 * Here is the struct for the dot we're going to be moving around on the
 * screen.  It has some constants to define its dimensions and velocity. It has
 * an a related event handler, a function to move it every frame, and a
 * function to render it. As for data members, it has variables for its x/y
 * position and x/y velocity.
 */
typedef struct {
	int mPosX, mPosY;
	int mVelX, mVelY;
} Dot;

SDL_Window* gWindow = NULL;
SDL_Renderer* gRenderer = NULL;
SDL_GameController* gGameController = NULL;
LTexture gDotTexture;

short init(void)
{
	if(SDL_Init(SDL_INIT_VIDEO | SDL_INIT_GAMECONTROLLER) < 0) {
		SDL_Log("%s(), SDL_Init failed. %s", __func__, SDL_GetError());
		return -1;
	}

    for (int i = 0; i < SDL_NumJoysticks(); i++) {
        if (SDL_IsGameController(i)) {
            gGameController = SDL_GameControllerOpen(i);
			if(gGameController == NULL) {
				SDL_Log("%s(), SDL_JoystickOpen failed. %s", __func__, SDL_GetError());
			}
			break;
        }
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
					SDL_RENDERER_ACCELERATED);
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

/*
 * In the rendering function we render the dot texture at the dot's position. 
 */
short LTexture_render(LTexture *lt, int x, int y, SDL_Rect* clip)
{
	SDL_Rect renderQuad = {x, y, lt->mWidth, lt->mHeight};

	if(clip != NULL) {
		renderQuad.w = clip->w;
		renderQuad.h = clip->h;
	}

	return SDL_RenderCopy(gRenderer, lt->mTexture, clip, &renderQuad);
}

/*
 * The constructor simply initializes the variables.
 */
Dot *Dot_init(Dot *d)
{
	d->mPosX = 0;
	d->mPosY = 0;
	d->mVelX = 0;
	d->mVelY = 0;
	return d;
}

/*
 * In our event handler we're going to set the velocity based on the key press.
 *
 * You may be wondering why we don't simply just increase the positon when we
 * press the key. If we were to say add to the x position every time we press
 * the right key, we would have to repeatedly press the right key to keep it
 * moving. By setting the velocity, we just have to press the key once.
 *
 * If you're wondering why we're checking if the key repeat is 0, it's because
 * key repeat is enabled by default and if you press and hold a key it will
 * report multiple key presses. That means we have to check if the key press is
 * the first one because we only care when the key was first pressed.
 *
 * For those of you who haven't studied physics yet, velocity is the
 * speed/direction of an object. If an object is moving right at 10 pixels per
 * frame, it has a velocity of 10. If it is moving to the left at 10 pixel per
 * frame, it has a velocity of -10. If the dot's velocity is 10, this means
 * after 10 frames it will have moved 100 pixels over.
 */
void handle_keyboard_events(SDL_Event *e, Dot *d)
{
	if(e->type == SDL_KEYDOWN && e->key.repeat == 0)
	{
		switch(e->key.keysym.sym)
		{
			case SDLK_UP: d->mVelY -= DOT_VEL; break;
			case SDLK_DOWN: d->mVelY += DOT_VEL; break;
			case SDLK_LEFT: d->mVelX -= DOT_VEL; break;
			case SDLK_RIGHT: d->mVelX += DOT_VEL; break;
		}
	}
/*
 * When we release a key, we have to undo the velocity change when first
 * pressed it. When we pressed right key, we added to the x velocity. When we
 * release the right key here, we subtract from the x velocity to return it to
 * 0.
 */
	else if(e->type == SDL_KEYUP && e->key.repeat == 0)
	{
		switch(e->key.keysym.sym)
		{
			case SDLK_UP: d->mVelY += DOT_VEL; break;
			case SDLK_DOWN:d-> mVelY -= DOT_VEL; break;
			case SDLK_LEFT: d->mVelX += DOT_VEL; break;
			case SDLK_RIGHT: d->mVelX -= DOT_VEL; break;
		}
	}
}

/*
 * The "which" variable says which controller the axis motion came from, and
 * here we check that the event came from joystick 0.
 *
 * We want to check whether it was a motion in the x direction or y
 * direction, which the "axis" variable indicates. Typically, axis 0 is the x
 * axis.
 *
 * The "value" variable says what position the analog stick has on the axis. If
 * the x position is less than the dead zone, the direction is set to negative.
 * If the position is greater than the dead zone, the direction is set to
 * positive. If it's in the dead zone, the direction is set to 0.
 */
void handle_axis_motion(SDL_Event *e, Dot *d) {
	if(e->jaxis.which == 0) {
		if(e->jaxis.axis == 0)
		{
			if(e->jaxis.value < -JOYSTICK_DEAD_ZONE)
				d->mVelX -= DOT_JOY_VEL;
			else if(e->jaxis.value > JOYSTICK_DEAD_ZONE)
				d->mVelX += DOT_JOY_VEL;
			else
				d->mVelX = 0;
		}
/*
 * Here we do the same thing again with the y axis, which is identified with
 * the axis ID 1.
 */
		else if(e->jaxis.axis == 1)
		{
			if(e->jaxis.value < -JOYSTICK_DEAD_ZONE)
				d->mVelY -= DOT_JOY_VEL;
			else if(e->jaxis.value > JOYSTICK_DEAD_ZONE)
				d->mVelY += DOT_JOY_VEL;
			else
				d->mVelY = 0;
		}
	}
}

/*
 * Here's the function we call every frame to move the dot.
 *
 * First we move the dot along the x axis based on its x velocity. After that
 * we check if the dot moved off the screen. If it did, we then undo the
 * movement along the x axis.
 */
void Dot_move(Dot *d)
{
	d->mPosX += d->mVelX;

	if((d->mPosX < 0) || (d->mPosX + DOT_WIDTH > SCREEN_WIDTH))
		d->mPosX -= d->mVelX;

	d->mPosY += d->mVelY;

	if((d->mPosY < 0) || (d->mPosY + DOT_HEIGHT > SCREEN_HEIGHT))
		d->mPosY -= d->mVelY;
}

void Dot_render(Dot *d)
{
	LTexture_render(&gDotTexture, d->mPosX, d->mPosY, NULL);
}

short loadMedia(void)
{
	if(LTexture_loadFromFile(&gDotTexture, "dot.bmp"))
		return -1;
	return 0;
}

void close_all(void)
{
	free_texture(&gDotTexture);

	SDL_GameControllerClose(gGameController);
	gGameController = NULL;

	SDL_DestroyRenderer(gRenderer);
	SDL_DestroyWindow(gWindow);
	gWindow = NULL;
	gRenderer = NULL;

	IMG_Quit();
	SDL_Quit();
}

/*
 * Before we enter the main loop we declare a dot object.
 *
 * Finally here we use our dot in the main loop. In the event loop we handle
 * events for the dot. After that we update the dot's position and then render
 * it to the screen.
 *
 * Now in this tutorial we're basing the velocity as amount moved per frame. In
 * most games, the velocity is done per second. The reason were doing it per
 * frame is that it is easier, but if you know physics it shouldn't be hard to
 * update the dot's position based on time. If you don't know physics, just
 * stick with per frame velocity for now.
 */
int main(int argc, char* args[])
{
	if(init())
		goto equit;

	if(loadMedia())
		goto equit;

	SDL_Event e;
	Dot dot;
	Dot_init(&dot);

	while(1)
	{
		while(SDL_PollEvent(&e) != 0)
		{
			switch (e.type) {
			case SDL_QUIT:
				goto equit;
			case SDL_CONTROLLERAXISMOTION:
				handle_axis_motion(&e, &dot);
				break;
			case SDL_CONTROLLERDEVICEADDED:
				if (!gGameController) {
					gGameController = SDL_GameControllerOpen(e.cdevice.which);
				}
				break;
			case SDL_CONTROLLERDEVICEREMOVED:
				if (gGameController && e.cdevice.which == SDL_JoystickInstanceID(
						SDL_GameControllerGetJoystick(gGameController))) {
					SDL_GameControllerClose(gGameController);
					gGameController = NULL;
				}
				break;
			default:
				handle_keyboard_events(&e, &dot);
			}
		}

		Dot_move(&dot);

		SDL_SetRenderDrawColor(gRenderer, 0xFF, 0xFF, 0xFF, 0xFF);
		SDL_RenderClear(gRenderer);

		Dot_render(&dot);

		SDL_RenderPresent(gRenderer);
		SDL_Delay(16);
	}
equit:
	close_all();

	return 0;
}
