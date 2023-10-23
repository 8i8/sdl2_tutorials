/*
 * Force Feedback
 *
 * Now that we know how to how to use joysticks with SDL, we can now use the
 * new haptics API to make the controller shake.
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
LTexture gSplashTexture;

/*
 * A haptic device is something that gives some sort of physical feedback. In
 * this case, it makes the controller rumble. The datatype for a haptics device
 * is intuitively named SDL_haptic.
 */
SDL_GameController* gGameController = NULL;
SDL_Haptic* gControllerHaptic = NULL;

/*
 * Like with the joysticks subsystem, you need to make sure to initialize the
 * haptic specific subsystem in order to use haptics.
 */
short init(void)
{
	//if(SDL_Init(SDL_INIT_VIDEO | SDL_INIT_GAMECONTROLLER | SDL_INIT_HAPTIC)) {
	if(SDL_Init(SDL_INIT_VIDEO | SDL_INIT_GAMECONTROLLER)) {
		SDL_Log("%s(), SDL_Init failed. %s", __func__, SDL_GetError());
		return -1;
	}

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
/*/1* */
/* * After we initialize the joystick, we need to get the haptics device from the */
/* * joystick using SDL_HapticOpenFromJoystick on an opened joystick. If we */
/* * manage to get the haptic device from controller we have to initialize the */
/* * rumble using SDL_HapticRumbleInit. */
/* *1/ */
/*	gControllerHaptic = SDL_HapticOpenFromJoystick( */
/*						SDL_GameControllerGetJoystick(gGameController)); */
/*	if(gControllerHaptic == NULL) { */
/*		SDL_Log("%s(), SDL_HapticOpenFromJoystick failed. %s", __func__, SDL_GetError()); */
/*		return -1; */
/*	} */

/*	if(SDL_HapticRumbleInit(gControllerHaptic) < 0) { */
/*		SDL_Log("%s(), SDL_HapticRumbleInit failed. %s", __func__, SDL_GetError()); */
/*		return -1; */
/*	} */

	gWindow = SDL_CreateWindow(
					"SDL Tutorial",
					SDL_WINDOWPOS_UNDEFINED,
					SDL_WINDOWPOS_UNDEFINED,
					SCREEN_WIDTH, SCREEN_HEIGHT,
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
	if(lt->mTexture != NULL) {
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

short loadMedia(void)
{
	if(LTexture_loadFromFile(&gSplashTexture, "splash.png"))
		return -1;

	return 0;
}

/*
 * Once we're done with a haptic device, we call SDL_HapticClose.
 */
void close_all(void)
{
	free_texture(&gSplashTexture);

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
 * To actually make the controller rumble, you need to make it play some sort
 * of rumbling. The easiest way to make your controller shake is by calling
 * SDL_HapticRumblePlay, which takes in the haptic device, strength in
 * percentage, and duration of the rumble. Here we make the controller rumble
 * at 75% strength for half a second whenever a SDL_JoyButtonEvent happens.
 *
 * Now the SDL 2 haptics API has many more features not covered here including
 * making custom effects, handling multi rumble devices, and handling haptic
 * mice. You can check them out in the SDL 2 force feedback documentation.
 */
void joystic_rumble(void)
{
	/* if(SDL_HapticRumblePlay(gControllerHaptic, 0.75, 500)) */
	/* 	SDL_Log("%s(), SDL_HapticRumblePlay failed. %s", */
	/* 			__func__, SDL_GetError()); */
	if (SDL_GameControllerHasRumble(gGameController))
		SDL_GameControllerRumble(gGameController, 0xFFFF, 0xFFFF, 1000);
	else
		SDL_Log("%s(), no rumble on controller. %s", __func__, SDL_GetError());
}

int main(void)
{
	SDL_Event e;
			
	if(init())
		goto equit;

	if(loadMedia())
		goto equit;

	while(1)
	{
		while(SDL_PollEvent(&e) != 0) {
			if( e.type == SDL_QUIT )
				goto equit;

			if(e.type == SDL_JOYBUTTONDOWN)
				joystic_rumble();
		}

		SDL_SetRenderDrawColor(gRenderer, 0xFF, 0xFF, 0xFF, 0xFF);
		SDL_RenderClear(gRenderer);

		LTexture_render(&gSplashTexture, 0, 0, NULL, 0.0, NULL,
						SDL_FLIP_NONE);

		SDL_RenderPresent(gRenderer);
	}
equit:
	close_all();

	return 0;
}
