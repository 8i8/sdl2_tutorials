#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <stdio.h>

#define SCREEN_WIDTH	640
#define SCREEN_HEIGHT	480

#define DOT_WIDTH	20
#define DOT_HEIGHT	20
#define DOT_VEL		10

#define TOTAL_PARTICLES	40
#define P_LIFE		10

typedef struct {
	SDL_Texture* mTexture;
	int mWidth;
	int mHeight;
} LTexture;

typedef struct {
	int mPosX, mPosY;
	int mFrame;
	LTexture *mTexture;
} Particle;

typedef struct {
	Particle* particles[TOTAL_PARTICLES];
	int mPosX, mPosY;
	int mVelX, mVelY;
} Dot;

SDL_Window* gWindow = NULL;
SDL_Renderer* gRenderer = NULL;
LTexture gDotTexture;
LTexture gRedTexture;
LTexture gGreenTexture;
LTexture gBlueTexture;
LTexture gShimmerTexture;

short init()
{
	if(SDL_Init(SDL_INIT_VIDEO) < 0) {
		SDL_Log("%s(), SDL_Init failed.", __func__);
		return -1;
	}

	if(SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "1") == 0)
		SDL_Log("Warning: Linear texture filtering not enabled!");

	gWindow = SDL_CreateWindow(
					"SDL Tutorial",
					SDL_WINDOWPOS_UNDEFINED,
					SDL_WINDOWPOS_UNDEFINED,
					SCREEN_WIDTH,
					SCREEN_HEIGHT,
					SDL_WINDOW_SHOWN);
	if(gWindow == NULL) {
		SDL_Log("%s(), SDL_CreateWindow failed.", __func__);
		return -1;
	}

	gRenderer = SDL_CreateRenderer(
					gWindow,
					-1,
					SDL_RENDERER_ACCELERATED
					| SDL_RENDERER_PRESENTVSYNC);
	if(gRenderer == NULL) {
		SDL_Log("%s(), SDL_CreateRenderer failed.", __func__);
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
		SDL_Log("%s(), IMG_Load failed to load \"%s\".",
				__func__, path);
		return -1;
	}

	SDL_SetColorKey(
			loadedSurface,
			SDL_TRUE,
			SDL_MapRGB(loadedSurface->format, 0, 0xFF, 0xFF));

	newTexture = SDL_CreateTextureFromSurface(gRenderer, loadedSurface);
	if(newTexture == NULL) {
		SDL_Log("%s(), SDL_CreateTextureFromSurface failed.", __func__);
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

void Particle_render(Particle *p)
{
	LTexture_render(p->mTexture, p->mPosX, p->mPosY, NULL);

	if(p->mFrame % 2 == 0)
		LTexture_render(&gShimmerTexture, p->mPosX, p->mPosY, NULL);

	p->mFrame++;
}

short Particle_isDead(Particle *p)
{
	return p->mFrame > P_LIFE;
}

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

void Dot_handleEvent(Dot *d, SDL_Event *e)
{
	if(e->type == SDL_KEYDOWN && e->key.repeat == 0)
	{
		switch(e->key.keysym.sym) {
			case SDLK_UP: 	d->mVelY -= DOT_VEL; break;
			case SDLK_DOWN: d->mVelY += DOT_VEL; break;
			case SDLK_LEFT: d->mVelX -= DOT_VEL; break;
			case SDLK_RIGHT:d->mVelX += DOT_VEL; break;
		}
	}
	else if(e->type == SDL_KEYUP && e->key.repeat == 0)
	{
		switch(e->key.keysym.sym) {
			case SDLK_UP: 	d->mVelY += DOT_VEL; break;
			case SDLK_DOWN: d->mVelY -= DOT_VEL; break;
			case SDLK_LEFT: d->mVelX += DOT_VEL; break;
			case SDLK_RIGHT:d->mVelX -= DOT_VEL; break;
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

short loadMedia()
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

int main(int argc, char* argv[])
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
		while(SDL_PollEvent(&e) != 0) {
			if(e.type == SDL_QUIT)
				goto equit;

			Dot_handleEvent(&dot, &e);
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
