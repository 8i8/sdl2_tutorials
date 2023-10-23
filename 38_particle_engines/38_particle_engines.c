/*
 * Particle Engines
 *
 * Particles are just mini-animations; What we're going to do is take these
 * animations and spawn them around a dot to create a trail of colored
 * shimmering particles.
 */
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>

#define SCREEN_WIDTH	640
#define SCREEN_HEIGHT	480

#define DOT_WIDTH	20
#define DOT_HEIGHT	20
#define DOT_VEL		10
#define DOT_JOY_VEL		1
#define JOYSTICK_DEAD_ZONE	10000

#define TOTAL_PARTICLES	40
#define P_LIFE		10

typedef struct {
	SDL_Texture* mTexture;
	int mWidth;
	int mHeight;
} LTexture;

/*
 * Here is a simple particle struct. We have related functions, a constructor
 * to set the position, and a function to render it, a function to tell if the
 * particle is dead. In terms of data members we have a position, a frame of
 * animation, and a texture we'll render with.
 */
typedef struct {
	int mPosX, mPosY;
	int mFrame;
	LTexture *mTexture;
} Particle;

/*
 * Here is our dot with an array of particles and a function to render the
 * particles on the dot.
 */
typedef struct {
	Particle* particles[TOTAL_PARTICLES];
	int mPosX, mPosY;
	int mVelX, mVelY;
} Dot;

SDL_Window* gWindow = NULL;
SDL_Renderer* gRenderer = NULL;
SDL_GameController* gGameController = NULL;
LTexture gDotTexture;
LTexture gRedTexture;
LTexture gGreenTexture;
LTexture gBlueTexture;
LTexture gShimmerTexture;

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

	if(SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "1") == 0)
		SDL_Log("Warning: Linear texture filtering not enabled.");

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

void LTexture_free(LTexture *lt)
{
	if(lt->mTexture != NULL) {
		SDL_DestroyTexture(lt->mTexture);
		lt->mTexture = NULL;
		lt->mWidth = 0;
		lt->mHeight = 0;
	}
}

short LTexture_loadFromFile(LTexture *lt, char *path)
{
	LTexture_free(lt);

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

void LTexture_setAlpha(LTexture *lt, Uint8 alpha)
{
	SDL_SetTextureAlphaMod(lt->mTexture, alpha);
}

void LTexture_render(
			LTexture *lt,
			int x,
			int y,
			SDL_Rect* clip)
{
	SDL_Rect renderQuad = { x, y, lt->mWidth, lt->mHeight };

	if(clip != NULL) {
		renderQuad.w = clip->w;
		renderQuad.h = clip->h;
	}

	SDL_RenderCopy(
			gRenderer,
			lt->mTexture,
			clip,
			&renderQuad);
}

/*
 * For our particle constructor we initialize the position around the given
 * position with some randomness to it. We then initialize the frame of
 * animation with some randomness so the particles will have varying life.
 * Finally we pick the type of texture we'll use for the particle also at
 * random.
 */
void Particle_init(Particle *p, int x, int y)
{
	srand(SDL_GetTicks());

	p->mPosX = x - 5 + (rand() % 25);
	p->mPosY = y - 5 + (rand() % 25);

	p->mFrame = rand() % 5;

	switch(rand() % 3)
	{
		case 0: p->mTexture = &gRedTexture; break;
		case 1: p->mTexture = &gGreenTexture; break;
		case 2: p->mTexture = &gBlueTexture; break;
	}
}

/*
 * In the rendering function we render our texture selected in the constructor
 * and then every other frame we render a semitransparent shimmer texture over
 * it to make it look like the particle is shining. We then update the frame of
 * animation.
 */
void Particle_render(Particle *p)
{
	LTexture_render(p->mTexture, p->mPosX, p->mPosY, NULL);

	if(p->mFrame % 2 == 0)
		LTexture_render(&gShimmerTexture, p->mPosX, p->mPosY, NULL);

	p->mFrame++;
}

/*
 * Once the particle has rendered for a max of 10 frames, we mark it as dead.
 */
short Particle_isDead(Particle *p)
{
	return p->mFrame > P_LIFE;
}

/*
 * The constructor/destructor now have to allocate/deallocate the particles we
 * render about the dot.
 */
void Dot_init(Dot *d)
{
	int i;
	d->mPosX = 0;
	d->mPosY = 0;

	d->mVelX = 0;
	d->mVelY = 0;

	for(i = 0; i < TOTAL_PARTICLES; ++i) {
		d->particles[i] = malloc(sizeof(Particle));
		Particle_init(d->particles[i], d->mPosX, d->mPosY);
	}
}

void Dot_free(Dot *d)
{
	int i;
	for(i = 0; i < TOTAL_PARTICLES; ++i)
		free(d->particles[i]);
}

void handle_keyboard_events(Dot *d, SDL_Event *e)
{
	if(e->type == SDL_KEYDOWN && e->key.repeat == 0)
	{
		switch(e->key.keysym.sym) {
			case SDLK_UP:	d->mVelY -= DOT_VEL; break;
			case SDLK_DOWN: d->mVelY += DOT_VEL; break;
			case SDLK_LEFT: d->mVelX -= DOT_VEL; break;
			case SDLK_RIGHT:d->mVelX += DOT_VEL; break;
		}
	}
	else if(e->type == SDL_KEYUP && e->key.repeat == 0)
	{
		switch(e->key.keysym.sym) {
			case SDLK_UP:	d->mVelY += DOT_VEL; break;
			case SDLK_DOWN: d->mVelY -= DOT_VEL; break;
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

void Dot_move(Dot *d)
{
	d->mPosX += d->mVelX;

	if((d->mPosX < 0) || (d->mPosX + DOT_WIDTH > SCREEN_WIDTH))
		d->mPosX -= d->mVelX;

	d->mPosY += d->mVelY;

	if((d->mPosY < 0) || (d->mPosY + DOT_HEIGHT > SCREEN_HEIGHT))
		d->mPosY -= d->mVelY;
}

/*
 * Our dot's rendering function now calls our particle rendering function. The
 * particle rendering function checks if there is any particles that are dead
 * and replaces them. After the dead particles are replaced we render all the
 * current particles to the screen.
 */
void Dot_renderParticles(Dot *d)
{
	int i;
	for(i = 0; i < TOTAL_PARTICLES; ++i) {
		if(Particle_isDead(d->particles[i])) {
			Particle_init(d->particles[i], d->mPosX, d->mPosY);
		}
		Particle_render(d->particles[i]);
	}
}

void Dot_render(Dot *d)
{
	LTexture_render(&gDotTexture, d->mPosX, d->mPosY, NULL);

	Dot_renderParticles(d);
}

/*
 * To give our particles a semi transparent look we set their alpha to 192.
 */
short loadMedia(void)
{
	if(LTexture_loadFromFile(&gDotTexture, "dot.bmp") < 0)
		return -1;

	if(LTexture_loadFromFile(&gRedTexture, "red.bmp") < 0)
		return -1;

	if(LTexture_loadFromFile(&gGreenTexture, "green.bmp") < 0)
		return -1;

	if(LTexture_loadFromFile(&gBlueTexture, "blue.bmp") < 0)
		return -1;

	if(LTexture_loadFromFile(&gShimmerTexture, "shimmer.bmp") < 0)
		return -1;

	LTexture_setAlpha(&gRedTexture, 192);
	LTexture_setAlpha(&gGreenTexture, 192);
	LTexture_setAlpha(&gBlueTexture, 192);
	LTexture_setAlpha(&gShimmerTexture, 192);

	return 0;
}

void close_all(Dot *d)
{
	Dot_free(d);

	SDL_GameControllerClose(gGameController);
	gGameController = NULL;

	LTexture_free(&gDotTexture);
	LTexture_free(&gRedTexture);
	LTexture_free(&gGreenTexture);
	LTexture_free(&gBlueTexture);
	LTexture_free(&gShimmerTexture);

	SDL_DestroyRenderer(gRenderer);
	SDL_DestroyWindow(gWindow);
	gWindow = NULL;
	gRenderer = NULL;

	IMG_Quit();
	SDL_Quit();
}

/*
 * Again, since our code is well encapsulated the code in the main loop hardly
 * changes.
 *
 * Now like most of the tutorials this is a super simplified example. In larger
 * program there would be particles controlled by a particle emitter that's its
 * own class, but for the sake of simplicity we're having the Dot class
 * function as a particle emitter.
 */
int main(void)
{
	SDL_Event e;
	Dot dot;

	if(init())
		goto equit;

	if(loadMedia())
		goto equit;

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
				handle_keyboard_events(&dot, &e);
			}
		}

		Dot_move(&dot);

		SDL_SetRenderDrawColor(gRenderer, 0xFF, 0xFF, 0xFF, 0xFF);
		SDL_RenderClear(gRenderer);

		Dot_render(&dot);

		SDL_RenderPresent(gRenderer);
	}
equit:
	close_all(&dot);

	return 0;
}
