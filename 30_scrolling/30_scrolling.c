/*
 * Scrolling
 *
 * Up until now we've only been deal with levels the size of the screen. With
 * scrolling you can navigate through levels of any size by rendering
 * everything relative to a camera.
 *
 * The basic principle of scrolling is that you have a rectangle that functions
 * as a camera:
 *
 * And then you only render what's in the camera, which usually involves
 * rendering things relative to the camera or only showing portions of objects
 * inside the camera.
 */
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>

/*
 * Since the level is no longer the size of the screen we have to have a
 * separate set of constants to define the level size.
 */
#define LEVEL_WIDTH		1280
#define LEVEL_HEIGHT	960

#define SCREEN_WIDTH	640
#define SCREEN_HEIGHT	480

#define DOT_WIDTH		20
#define DOT_HEIGHT		20
#define DOT_VEL			5
#define DOT_JOY_VEL		1
#define JOYSTICK_DEAD_ZONE	10000

typedef struct {
	SDL_Texture *mTexture;
	int mWidth;
	int mHeight;
} LTexture;

typedef struct {
	int mPosX, mPosY;
	int mVelX, mVelY;
} Dot;

SDL_Window* gWindow = NULL;
SDL_Renderer* gRenderer = NULL;
SDL_GameController* gGameController = NULL;
LTexture gDotTexture;
LTexture gBGTexture;

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

short LTexture_render(LTexture *lt, int x, int y, SDL_Rect* clip)
{
	SDL_Rect renderQuad = {x, y, lt->mWidth, lt->mHeight};

	if(clip != NULL) {
		renderQuad.w = clip->w;
		renderQuad.h = clip->h;
	}

	return SDL_RenderCopy(gRenderer, lt->mTexture, clip, &renderQuad);
}

void Dot_init(Dot *d)
{
	d->mPosX = 0;
	d->mPosY = 0;

	d->mVelX = 0;
	d->mVelY = 0;
}

void handle_keyboard_events(Dot *d, SDL_Event *e)
{
	if(e->type == SDL_KEYDOWN && e->key.repeat == 0)
	{
		switch(e->key.keysym.sym)
		{
			case SDLK_UP: 	d->mVelY -= DOT_VEL; break;
			case SDLK_DOWN: d->mVelY += DOT_VEL; break;
			case SDLK_LEFT: d->mVelX -= DOT_VEL; break;
			case SDLK_RIGHT:d->mVelX += DOT_VEL; break;
		}
	}
	else if(e->type == SDL_KEYUP && e->key.repeat == 0)
	{
		switch(e->key.keysym.sym)
		{
			case SDLK_UP:	d->mVelY += DOT_VEL; break;
			case SDLK_DOWN:	d->mVelY -= DOT_VEL; break;
			case SDLK_LEFT: d->mVelX += DOT_VEL; break;
			case SDLK_RIGHT:d->mVelX -= DOT_VEL; break;
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
 * This time when moving the dot, we check if the dot moved off the level as
 * opposed to checking if it moved off the screen since the screen is going to
 * move around the level.
 */
void Dot_move(Dot *d)
{
	d->mPosX += d->mVelX;

	if((d->mPosX < 0) || (d->mPosX + DOT_WIDTH > LEVEL_WIDTH))
		d->mPosX -= d->mVelX;

	d->mPosY += d->mVelY;

	if((d->mPosY < 0) || (d->mPosY + DOT_HEIGHT > LEVEL_HEIGHT))
		d->mPosY -= d->mVelY;
}

/*
 * This time the dot has to render relative to the camera, so its rendering
 * function takes in a camera position.
 *
 * Now when we render objects to the screen, we render them relative to the
 * camera by subtracting the camera offset.
 */
void Dot_render(Dot *d, int camX, int camY)
{
	LTexture_render(&gDotTexture, d->mPosX - camX, d->mPosY - camY, NULL);
}

short loadMedia(void)
{
	if(LTexture_loadFromFile(&gDotTexture, "dot.bmp") < 0)
		return -1;

	if(LTexture_loadFromFile(&gBGTexture, "bg.png") < 0)
		return -1;

	return 0;
}

void close_all(void)
{
	free_texture(&gDotTexture);
	free_texture(&gBGTexture);

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
 * Before we go into the main loop, we declare the dot and the camera that is
 * going to be following it.
 */
int main(int argc, char* argv[])
{
	if(init())
		goto equit;

	if(loadMedia())
		goto equit;

	SDL_Event e;
	Dot dot;
	Dot_init(&dot);
	SDL_Rect camera = { 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT };

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
				handle_keyboard_events(&dot, &e);
			}
		}

		Dot_move(&dot);
/*
 * After we move the dot, we want to change the camera position to center over
 * it. We don't want the camera to go outside of the level so we keep it in
 * bounds after moving it.
 */
		camera.x = (dot.mPosX + DOT_WIDTH / 2) - SCREEN_WIDTH / 2;
		camera.y = (dot.mPosY + DOT_HEIGHT / 2) - SCREEN_HEIGHT / 2;

		if(camera.x < 0)
		       camera.x = 0;
		if(camera.y < 0)
		       camera.y = 0;
		if(camera.x > LEVEL_WIDTH - camera.w)
		       camera.x = LEVEL_WIDTH - camera.w;
		if(camera.y > LEVEL_HEIGHT - camera.h)
			camera.y = LEVEL_HEIGHT - camera.h;
/*
 * After the camera is in place we render the portion of the background that is
 * inside that camera and then render the dot relative to the camera position.
 */
		SDL_SetRenderDrawColor(gRenderer, 0xFF, 0xFF, 0xFF, 0xFF);
		SDL_RenderClear(gRenderer);

		LTexture_render(&gBGTexture, 0, 0, &camera);

		Dot_render(&dot, camera.x, camera.y);

		SDL_RenderPresent(gRenderer);
	}
equit:
	close_all();

	return 0;
}

