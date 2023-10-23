/*
 * Gamepads and Joysticks
 *
 * Just like with mouse input and keyboard input, SDL has the ability to read
 * input from a joystick/gamepad/game controller. In this tutorial we'll be
 * making an arrow rotate based on the input of a joystick.
 */
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>

#define SCREEN_WIDTH		640
#define SCREEN_HEIGHT		480

/*
 * The way SDL handles analog sticks on a game controller is that it converts
 * its position into a number between -32768 to 32767. This mean a light tap
 * could report a position of 1000+. We want to ignore light taps, so we want
 * to create a dead zone where input from the joystick is ignored. This is why
 * we define this constant and we'll see how it works later.
 */
#define JOYSTICK_DEAD_ZONE	8000

typedef struct {
	SDL_Texture *mTexture;
	int mWidth;
	int mHeight;
} LTexture;

SDL_Window* gWindow = NULL;
SDL_Renderer* gRenderer = NULL;
LTexture gArrowTexture;

/*
 * The data type for a game controller is SDL_GameController. Here we declare
 * the global controller handle we'll use to interact with the game controller.
 */
SDL_GameController* gGameController = NULL;

SDL_GameController *findController(void) {
    for (int i = 0; i < SDL_NumJoysticks(); i++) {
        if (SDL_IsGameController(i)) {
            return SDL_GameControllerOpen(i);
        }
    }

    return NULL;
}

/*
 * This is important.
 *
 * Up until now, we've been only initializing video so we can render to the
 * screen. Now we need to initialize the joystick subsystem or reading from
 * joystick won't work.
 */
short init(void)
{
	if(SDL_Init(SDL_INIT_VIDEO | SDL_INIT_GAMECONTROLLER) < 0) {
		SDL_Log("%s(), SDL_Init failed. %s", __func__, SDL_GetError());
		return -1;
	}

	if(SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "2") == 0)
		SDL_Log("Warning: Linear texture filtering disabled.");
/*
 * After initializing the joystick subsystem, we want to open our joystick.
 * First we call SDL_NumJoysticks to check if there is at least one joystick
 * connected. If there is, we call SDL_JoystickOpen to open the joystick at
 * index 0. After the joystick is open, it will now report events to the SDL
 * event queue.
 */
	if(SDL_NumJoysticks() < 1)
		SDL_Log("Warning: No input device connected.");
	else {
		for (int i = 0; i < SDL_NumJoysticks(); i++) {
			if (SDL_IsGameController(i)) {
				gGameController = SDL_GameControllerOpen(i);
				if(gGameController == NULL) {
					SDL_Log("%s(), SDL_JoystickOpen failed. %s", __func__, SDL_GetError());
					return -1;
				}
				break;
			}
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

	if(clip != NULL) {
		renderQuad.w = clip->w;
		renderQuad.h = clip->h;
	}

	return SDL_RenderCopyEx(gRenderer, lt->mTexture, clip,
				&renderQuad, angle, center, flip);
}

int LTexture_getWidth(LTexture *lt)
{
	return lt->mWidth;
}

int LTexture_getHeight(LTexture *lt)
{
	return lt->mHeight;
}

short loadMedia(void)
{
	if(LTexture_loadFromFile(&gArrowTexture, "arrow.png"))
		return -1;

	return 0;
}

/*
 * After we're done with the joystick, we close it with SDL_JoystickClose.
 */
void close_all(void)
{
	free_texture(&gArrowTexture);

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
void controller_input(SDL_Event *e, int *xDir, int *yDir)
{
	if(e->jaxis.which == 0) {
		if(e->jaxis.axis == 0)
		{
			if(e->jaxis.value < -JOYSTICK_DEAD_ZONE)
				*xDir = -1;
			else if(e->jaxis.value > JOYSTICK_DEAD_ZONE)
				*xDir =  1;
			else
				*xDir = 0;
		}
/*
 * Here we do the same thing again with the y axis, which is identified with
 * the axis ID 1.
 */
		else if(e->jaxis.axis == 1)
		{
			if(e->jaxis.value < -JOYSTICK_DEAD_ZONE)
				*yDir = -1;
			else if(e->jaxis.value > JOYSTICK_DEAD_ZONE)
				*yDir =  1;
			else
				*yDir = 0;
		}
	}
}

/*
 * For this demo, we want to keep track of the x and y direction. If the x
 * equals -1, the joystick's x position is pointing left. If it is +1, the x
 * position is pointing right. The y position for joysticks has positive being
 * up and negative being down, so y = +1 is up and y = -1 is down. If x or y is
 * 0, that means it's in the dead zone and is in the center.
 */
int main(int argc, char* argv[])
{
	SDL_Event e;
	int xDir = 0;
	int yDir = 0;
	double joystickAngle;

	if(init())
		goto equit;

	if(loadMedia())
		goto equit;

	while(1)
	{
/*
 * In our event loop, we check if the joystick has moved by checking for a
 * SDL_JoyAxisEvent.
 */
		while(SDL_PollEvent(&e) != 0)
		{
			switch (e.type) {
			case SDL_QUIT:
				goto equit;
			case SDL_CONTROLLERAXISMOTION:
				controller_input(&e, &xDir, &yDir);
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
			}
		}

		SDL_SetRenderDrawColor(gRenderer, 0xFF, 0xFF, 0xFF, 0xFF);
		SDL_RenderClear(gRenderer);
/*
 * Before we render the arrow which will point in the direction the the analog
 * stick is pushed, we need to calculate the angle. We do this using the cmath
 * function atan2, which stands for arc tangent 2, AKA inverse tangent 2.
 *
 * For those of you familiar with trigonometry, this is basically the inverse
 * tangent function with some additional code inside that takes into account
 * the which quadrant the values are coming from.
 *
 * For those of you only familiar with geometry, just know you give it the y
 * position and x position and it will give you the angle in radians. SDL wants
 * rotation angles in degrees, so we have to convert the radians to degrees by
 * multiplying it by 180 over Pi.
 *
 * When both the x and y position are 0, we could get a garbage angle, so we
 * correct the value to equal 0.
 */
		joystickAngle = atan2((float)yDir,(float)xDir) * (180.0 / M_PI);
		
		if(xDir == 0 && yDir == 0)
			joystickAngle = 0;
/*
 * Finally we render the arrow rotated on the screen.
 *
 * There are other joystick events like button presses, pov hats, and pluggin
 * in or removing a controller. They are fairly simple and you should be able
 * to pick them up with some look at the documentation and experimentation.
 */
		LTexture_render(
				&gArrowTexture,
				(SCREEN_WIDTH - LTexture_getWidth(
						&gArrowTexture)) / 2,
				(SCREEN_HEIGHT - LTexture_getHeight(
						&gArrowTexture)) / 2,
				NULL,
				joystickAngle,
				NULL,
				SDL_FLIP_NONE);

		SDL_RenderPresent(gRenderer);

		SDL_Delay(16);
	}
equit:
	close_all();

	return 0;
}

