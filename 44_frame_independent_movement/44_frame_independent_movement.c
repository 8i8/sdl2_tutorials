/*
 * Frame Independent Movement
 *
 * Whether you want to be able to handle unstable frame rates or support
 * multiple frame rates, you can set your movement based on time to make it
 * independent of frame rate.
 */
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <stdio.h>

#define SCREEN_WIDTH	640
#define SCREEN_HEIGHT	480
#define DOT_WIDTH	20
#define DOT_HEIGHT	20
#define DOT_VEL		300
#define DOT_JOY_VEL	50
#define JOYSTICK_DEAD_ZONE	10000

typedef struct {
	SDL_Texture* mTexture;
	void* mPixels;
	int mPitch;
	int mWidth;
	int mHeight;
} LTexture;

typedef struct {
	Uint32 mStartTicks;
	Uint32 mPausedTicks;
	short mPaused;
	short mStarted;
} LTimer;

/*
 * The dot struct returns adapted for frame independent movement. Notice how the
 * velocity is now 640. The way we did per frame velocity previously would
 * cause this to fly across the screen in a single frame. For this demo we're
 * going to base things on time and the standard unit of time is 1 second. 640
 * pixels per second translates into little more than 10 pixels per frame in a
 * 60 frames per second application.
 *
 * In order to move based on frame time, the move function needs to know how
 * much time is moving per frame. This is why the move function takes in a time
 * step which is how much time has passed since the last update.
 *
 * Also notice how the position and velocity are floats instead of integers. If
 * we used integers the motion would be always truncated to the nearest integer
 * which would cause greater inaccuracies.
 */
typedef struct {
	float mPosX, mPosY;
	float mVelX, mVelY;
} Dot;

SDL_Window* gWindow = NULL;
SDL_Renderer* gRenderer = NULL;
SDL_GameController* gGameController = NULL;
LTexture gDotTexture;
Dot dot;

void Dot_init(Dot *d)
{
	d->mPosX = 0;
	d->mPosY = 0;
	d->mVelX = 0;
	d->mVelY = 0;
}

short init(void)
{
	if(SDL_Init(SDL_INIT_VIDEO | SDL_INIT_GAMECONTROLLER) < 0) {
		SDL_Log("%s(), SDL_Init failed. %s", __func__, SDL_GetError());
		return -1;
	}

	if(SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "1") == 0) {
		SDL_Log("%s(), Warning: Linear texture filtering not enabled. %s", 
				__func__, SDL_GetError());
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

	gRenderer = SDL_CreateRenderer(gWindow, -1, SDL_RENDERER_ACCELERATED);
	if(gRenderer == NULL) {
		SDL_Log("%s(), SDL_CreateRenderer failed. %s", __func__, SDL_GetError());
		return -1;
	}

	SDL_SetRenderDrawColor(gRenderer, 0xFF, 0xFF, 0xFF, 0xFF);

	if((IMG_Init(IMG_INIT_PNG) & IMG_INIT_PNG) == 0) {
		SDL_Log("%s(), IMG_Init failed. %s", __func__, IMG_GetError());
		return -1;
	}

	Dot_init(&dot);

	return 0;
}

void LTexture_free(LTexture *lt)
{
	if(lt->mTexture != NULL) {
		SDL_DestroyTexture(lt->mTexture);
		lt->mTexture = NULL;
		lt->mWidth = 0;
		lt->mHeight = 0;
		lt->mPixels = NULL;
		lt->mPitch = 0;
	}
}

/*
 * vsync is not enabled for this tutorial.
 */
short LTexture_loadFromFile(LTexture *lt, char *path)
{
	LTexture_free(lt);

	SDL_Texture* newTexture = NULL;

	SDL_Surface* loadedSurface = IMG_Load(path);
	if(loadedSurface == NULL) {
		SDL_Log("%s(), IMG_Load failed. %s", __func__, IMG_GetError());

		return -1;
	}

	SDL_Surface* formattedSurface = SDL_ConvertSurfaceFormat(
							loadedSurface,
							SDL_PIXELFORMAT_RGBA8888,
							SDL_SWSURFACE);
	if(formattedSurface == NULL) {
		SDL_Log("%s(), SDL_ConvertSurfaceFormat failed.", __func__);
		return -1;
	}

	newTexture = SDL_CreateTexture(
					gRenderer,
					SDL_PIXELFORMAT_RGBA8888,
					SDL_TEXTUREACCESS_STREAMING,
					formattedSurface->w,
					formattedSurface->h);
	if(newTexture == NULL) {
		SDL_Log("%s(), SDL_CreateTextureFromSurface failed. %s", __func__, SDL_GetError());
		return -1;
	}

	SDL_SetTextureBlendMode(newTexture, SDL_BLENDMODE_BLEND);

	SDL_LockTexture(
				newTexture,
				&formattedSurface->clip_rect,
				&lt->mPixels,
				&lt->mPitch);

	memcpy(
			lt->mPixels,
			formattedSurface->pixels,
			formattedSurface->pitch * formattedSurface->h);

	lt->mWidth = formattedSurface->w;
	lt->mHeight = formattedSurface->h;

	Uint32* pixels = (Uint32*)lt->mPixels;
	int pixelCount = (lt->mPitch / 4) * lt->mHeight;

	Uint32 colorKey = SDL_MapRGB(formattedSurface->format, 0, 0xFF, 0xFF);
	Uint32 transparent = SDL_MapRGBA(
			formattedSurface->format, 0x00, 0xFF, 0xFF, 0x00);

	int i;
	for(i = 0; i < pixelCount; ++i)
		if(pixels[i] == colorKey)
			pixels[i] = transparent;

	SDL_UnlockTexture(newTexture);
	lt->mPixels = NULL;

	SDL_FreeSurface(formattedSurface);
		
	SDL_FreeSurface(loadedSurface);

	lt->mTexture = newTexture;

	return 0;
}

void LTexture_render(
			LTexture *lt,
			int x, int y,
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

	SDL_RenderCopyEx(gRenderer, lt->mTexture, clip,
				&renderQuad, angle, center, flip);
}

void LTimer_start(LTimer *t)
{
	t->mStarted = 1;
	t->mPaused = 0;
	t->mStartTicks = SDL_GetTicks();
	t->mPausedTicks = 0;
}

Uint32 LTimer_getTicks(LTimer *t)
{
	Uint32 time = 0;

	if(t->mStarted) {
		if(t->mPaused)
			time = t->mPausedTicks;
		else
			time = SDL_GetTicks() - t->mStartTicks;
	}

	return time;
}

/*
 * When we move around the dot we first get the time from the step time so we
 * know how much time has passed since the last time we moved. We turn it from
 * milliseconds into seconds and pass it to the move function. After we're done
 * moving we restart the step timer so we'll now how much time has passed for
 * when we need to move again. Finally we render as we normally do.
 *
 * For most of these tutorials, things are simplified to make things easier to
 * digest. For most if not all applications we use time based movement as
 * opposed to frame based movement. Even when we have a fixed frame rate, we
 * just use a constant time step. The thing is when using time based movement
 * you run into problems with floating point errors which require vector math
 * to fix, and vector math is beyond the scope of this tutorial which is why
 * frame based movement is used for most of the tutorials.
 */
void handle_keyboard_events(Dot *d, SDL_Event *e)
{
	if(e->type == SDL_KEYDOWN && e->key.repeat == 0) {
		switch(e->key.keysym.sym)
		{
			case SDLK_UP:    d->mVelY -= DOT_VEL; break;
			case SDLK_DOWN:  d->mVelY += DOT_VEL; break;
			case SDLK_LEFT:  d->mVelX -= DOT_VEL; break;
			case SDLK_RIGHT: d->mVelX += DOT_VEL; break;
		}
	}
	else if(e->type == SDL_KEYUP && e->key.repeat == 0) {
		switch(e->key.keysym.sym)
		{
			case SDLK_UP:    d->mVelY += DOT_VEL; break;
			case SDLK_DOWN:  d->mVelY -= DOT_VEL; break;
			case SDLK_LEFT:  d->mVelX += DOT_VEL; break;
			case SDLK_RIGHT: d->mVelX -= DOT_VEL; break;
		}
	}
}

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
 * Here is the move function changed for time based movement as opposed to
 * frame based movement.
 * 
 * We update the position by moving it over by velocity * time. Say if we had
 * (for simplicity's sake) a velocity of 600 pixels per second and a time step
 * of 1 60th of a second. This means we would move over by 600 * 1/60 pixels or
 * 10 pixels.
 * 
 * Because of the non uniform movement we can't just move back when we go off
 * screen, we have to correct the value to be something on screen.
 */
void Dot_move(Dot *d, float timeStep)
{
	d->mPosX += d->mVelX * timeStep;

	if(d->mPosX < 0)
		d->mPosX = 0;
	else if(d->mPosX > SCREEN_WIDTH - DOT_WIDTH)
		d->mPosX = SCREEN_WIDTH - DOT_WIDTH;
	
	d->mPosY += d->mVelY * timeStep;

	if(d->mPosY < 0)
		d->mPosY = 0;
	else if(d->mPosY > SCREEN_HEIGHT - DOT_HEIGHT)
		d->mPosY = SCREEN_HEIGHT - DOT_HEIGHT;
}

/*
 * To prevent the compiler from barking at us, we convert the positions to
 * integers when rendering the dot.
 */
void Dot_render(Dot *d)
{
	LTexture_render(&gDotTexture, (int)d->mPosX, (int)d->mPosY,
			NULL, 0.0, NULL, 0);
}

short loadMedia(void)
{
	if(LTexture_loadFromFile(&gDotTexture, "dot.bmp"))
		return -1;
	return 0;
}

void close_all(void)
{
	LTexture_free(&gDotTexture);

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
 * For this demo we disabled vsync to show it can run regardless of the frame
 * rate. In order to know how much time has passed between renders, we need to
 * timer to keep track of the time step.
 */
int main(int argc, char* args[])
{
	LTimer stepTimer;
	float timeStep;

	if(init())
		goto equit;

	if(loadMedia())
		goto equit;

	SDL_Event e;

	while(1)
	{
		while(SDL_PollEvent(&e) != 0) {
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
				handle_keyboard_events(&dot, &e);
			}
		}

		timeStep = LTimer_getTicks(&stepTimer) / 1000.f;
		Dot_move(&dot, timeStep);
		LTimer_start(&stepTimer);

		SDL_SetRenderDrawColor(gRenderer, 0xFF, 0xFF, 0xFF, 0xFF);
		SDL_RenderClear(gRenderer);
		Dot_render(&dot);
		SDL_RenderPresent(gRenderer);
		SDL_Delay(60);
	}
equit:
	close_all();

	return 0;
}
