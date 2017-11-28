/*
 * This program demonstrates the use of tiles in SDL.
 */
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <stdio.h>

#define SCREEN_WIDTH		640
#define SCREEN_HEIGHT		480
#define LEVEL_WIDTH		1280
#define LEVEL_HEIGHT		960
#define TILE_WIDTH		80
#define TILE_HEIGHT		80
#define TOTAL_TILES		192
#define TOTAL_TILE_SPRITES	12

#define DOT_WIDTH		20
#define DOT_HEIGHT		20
#define DOT_VEL			10

#define TILE_RED		0
#define TILE_GREEN		1
#define TILE_BLUE		2
#define TILE_CENTER		3
#define TILE_TOP		4
#define TILE_TOPRIGHT		5
#define TILE_RIGHT		6
#define TILE_BOTTOMRIGHT	7
#define TILE_BOTTOM		8
#define TILE_BOTTOMLEFT		9
#define TILE_LEFT		10
#define TILE_TOPLEFT		11

typedef struct {
	SDL_Texture* mTexture;
	int mWidth;
	int mHeight;
} LTexture;

typedef struct {
	SDL_Rect mBox;
	int mType;
} Tile;

typedef struct {
	SDL_Rect mBox;
	int mVelX, mVelY;
} Dot;

short checkCollision(SDL_Rect *a, SDL_Rect *b);
short touchesWall(SDL_Rect *box, Tile *tiles[]);
short setTiles(Tile *tiles[]);

SDL_Window* gWindow = NULL;
SDL_Renderer* gRenderer = NULL;
LTexture gDotTexture;
LTexture gTileTexture;
SDL_Rect gTileClips[TOTAL_TILE_SPRITES];

short init()
{
	if(SDL_Init(SDL_INIT_VIDEO) < 0) {
		SDL_Log("%s(), SDL_Init failed. %s", __func__, SDL_GetError());
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
		SDL_Log("%s(), SDL_CreateTextureFromSurface failed.",
				__func__);
		return -1;
	}

	lt->mWidth = loadedSurface->w;
	lt->mHeight = loadedSurface->h;

	SDL_FreeSurface(loadedSurface);

	lt->mTexture = newTexture;

	return 0;
}

void LTexture_render(
			LTexture *lt, 
			int x, int y,
			SDL_Rect* clip)
{
	SDL_Rect renderQuad = { x, y, lt->mWidth, lt->mHeight };

	if(clip != NULL) {
		renderQuad.w = clip->w;
		renderQuad.h = clip->h;
	}

	SDL_RenderCopy(gRenderer, lt->mTexture, clip, &renderQuad);
}

void Tile_init(Tile *t, int x, int y, int tileType)
{
	t->mBox.x = x;
	t->mBox.y = y;

	t->mBox.w = TILE_WIDTH;
	t->mBox.h = TILE_HEIGHT;

	t->mType = tileType;
}

void Tile_render(Tile *t, SDL_Rect *camera)
{
	if(checkCollision(camera, &t->mBox))
		LTexture_render(
				&gTileTexture,
				t->mBox.x - camera->x,
				t->mBox.y - camera->y,
				&gTileClips[t->mType]);
}

int Tile_getType(Tile *t)
{
	return t->mType;
}

SDL_Rect *Tile_getBox(Tile *t)
{
	return &t->mBox;
}

void Dot_init(Dot *d)
{
	d->mBox.x = 0;
	d->mBox.y = 0;
	d->mBox.w = DOT_WIDTH;
	d->mBox.h = DOT_HEIGHT;

	d->mVelX = 0;
	d->mVelY = 0;
}

void Dot_handleEvent(Dot *d, SDL_Event *e)
{
	if(e->type == SDL_KEYDOWN && e->key.repeat == 0)
	{
		switch(e->key.keysym.sym) {
			case SDLK_UP:    d->mVelY -= DOT_VEL; break;
			case SDLK_DOWN:  d->mVelY += DOT_VEL; break;
			case SDLK_LEFT:  d->mVelX -= DOT_VEL; break;
			case SDLK_RIGHT: d->mVelX += DOT_VEL; break;
		}
	}
	else if(e->type == SDL_KEYUP && e->key.repeat == 0)
	{
		switch(e->key.keysym.sym)
{
			case SDLK_UP:    d->mVelY += DOT_VEL; break;
			case SDLK_DOWN:  d->mVelY -= DOT_VEL; break;
			case SDLK_LEFT:  d->mVelX += DOT_VEL; break;
			case SDLK_RIGHT: d->mVelX -= DOT_VEL; break;
		}
	}
}

void Dot_move(Dot *d, Tile *tiles[])
{
	d->mBox.x += d->mVelX;

	if((d->mBox.x < 0) || (d->mBox.x + DOT_WIDTH > LEVEL_WIDTH)
			|| touchesWall(&d->mBox, tiles))
		d->mBox.x -= d->mVelX;

	d->mBox.y += d->mVelY;

	if((d->mBox.y < 0) || (d->mBox.y + DOT_HEIGHT > LEVEL_HEIGHT)
			|| touchesWall(&d->mBox, tiles))
		d->mBox.y -= d->mVelY;
}

void Dot_setCamera(Dot *d, SDL_Rect *camera)
{
	camera->x = (d->mBox.x + DOT_WIDTH / 2) - SCREEN_WIDTH / 2;
	camera->y = (d->mBox.y + DOT_HEIGHT / 2) - SCREEN_HEIGHT / 2;

	if(camera->x < 0)
		camera->x = 0;
	if(camera->y < 0)
		camera->y = 0;
	if(camera->x > LEVEL_WIDTH - camera->w)
		camera->x = LEVEL_WIDTH - camera->w;
	if(camera->y > LEVEL_HEIGHT - camera->h)
		camera->y = LEVEL_HEIGHT - camera->h;
}

void Dot_render(Dot *d, SDL_Rect *camera)
{
	LTexture_render(
			&gDotTexture,
			d->mBox.x - camera->x,
			d->mBox.y - camera->y,
			NULL);
}

short loadMedia(Tile* tiles[])
{
	if(LTexture_loadFromFile(&gDotTexture, "dot.bmp"))
		return -1;

	if(LTexture_loadFromFile(&gTileTexture, "tiles.png"))
		return -1;

	if(setTiles(tiles))
		return -1;

	return 0;
}

void close_all(Tile* tiles[])
{
	int i;
	for(i = 0; i < TOTAL_TILES; ++i)
		if(tiles[i] == NULL) {
			free(tiles[i]);
			tiles[i] = NULL;
		}

	LTexture_free(&gDotTexture);
	LTexture_free(&gTileTexture);

	SDL_DestroyRenderer(gRenderer);
	SDL_DestroyWindow(gWindow);
	gWindow = NULL;
	gRenderer = NULL;

	IMG_Quit();
	SDL_Quit();
}

short checkCollision(SDL_Rect *a, SDL_Rect *b)
{
	int leftA, leftB;
	int rightA, rightB;
	int topA, topB;
	int bottomA, bottomB;

	leftA = a->x;
	rightA = a->x + a->w;
	topA = a->y;
	bottomA = a->y + a->h;

	leftB = b->x;
	rightB = b->x + b->w;
	topB = b->y;
	bottomB = b->y + b->h;

	if(bottomA <= topB)
		return 0;
	if(topA >= bottomB)
		return 0;
	if(rightA <= leftB)
		return 0;
	if(leftA >= rightB)
		return 0;

	return 1;
}

short setTiles(Tile* tiles[])
{
	int x = 0, y = 0, i;
	int tile = -1;
	FILE *fp;

	fp = fopen("lazy.map", "r");

	if(fp == NULL) {
		SDL_Log("%s(), fopen failed.", __func__);
		return -1;
	}

	for(i = 0; i < TOTAL_TILES; ++i)
	{
		int *tileType;
		tileType = &tile;

		if(fscanf(fp, "%d", tileType) < 0) {
			SDL_Log("%s(), fscanf failed.", __func__);
			return -1;
		}

		if((*tileType >= 0) && (*tileType < TOTAL_TILE_SPRITES) == 0) {
			SDL_Log("%s(), invalid tile type.", __func__);
			return -1;
		}
		tiles[i] = malloc(sizeof(Tile));
		Tile_init(tiles[i], x, y, *tileType);
		
		x += TILE_WIDTH;

		if(x >= LEVEL_WIDTH) {
			x = 0;
			y += TILE_HEIGHT;
		}
	}
		
	gTileClips[TILE_RED].x = 0;
	gTileClips[TILE_RED].y = 0;
	gTileClips[TILE_RED].w = TILE_WIDTH;
	gTileClips[TILE_RED].h = TILE_HEIGHT;

	gTileClips[TILE_GREEN].x = 0;
	gTileClips[TILE_GREEN].y = 80;
	gTileClips[TILE_GREEN].w = TILE_WIDTH;
	gTileClips[TILE_GREEN].h = TILE_HEIGHT;

	gTileClips[TILE_BLUE].x = 0;
	gTileClips[TILE_BLUE].y = 160;
	gTileClips[TILE_BLUE].w = TILE_WIDTH;
	gTileClips[TILE_BLUE].h = TILE_HEIGHT;

	gTileClips[TILE_TOPLEFT].x = 80;
	gTileClips[TILE_TOPLEFT].y = 0;
	gTileClips[TILE_TOPLEFT].w = TILE_WIDTH;
	gTileClips[TILE_TOPLEFT].h = TILE_HEIGHT;

	gTileClips[TILE_LEFT].x = 80;
	gTileClips[TILE_LEFT].y = 80;
	gTileClips[TILE_LEFT].w = TILE_WIDTH;
	gTileClips[TILE_LEFT].h = TILE_HEIGHT;

	gTileClips[TILE_BOTTOMLEFT].x = 80;
	gTileClips[TILE_BOTTOMLEFT].y = 160;
	gTileClips[TILE_BOTTOMLEFT].w = TILE_WIDTH;
	gTileClips[TILE_BOTTOMLEFT].h = TILE_HEIGHT;

	gTileClips[TILE_TOP].x = 160;
	gTileClips[TILE_TOP].y = 0;
	gTileClips[TILE_TOP].w = TILE_WIDTH;
	gTileClips[TILE_TOP].h = TILE_HEIGHT;

	gTileClips[TILE_CENTER].x = 160;
	gTileClips[TILE_CENTER].y = 80;
	gTileClips[TILE_CENTER].w = TILE_WIDTH;
	gTileClips[TILE_CENTER].h = TILE_HEIGHT;

	gTileClips[TILE_BOTTOM].x = 160;
	gTileClips[TILE_BOTTOM].y = 160;
	gTileClips[TILE_BOTTOM].w = TILE_WIDTH;
	gTileClips[TILE_BOTTOM].h = TILE_HEIGHT;

	gTileClips[TILE_TOPRIGHT].x = 240;
	gTileClips[TILE_TOPRIGHT].y = 0;
	gTileClips[TILE_TOPRIGHT].w = TILE_WIDTH;
	gTileClips[TILE_TOPRIGHT].h = TILE_HEIGHT;

	gTileClips[TILE_RIGHT].x = 240;
	gTileClips[TILE_RIGHT].y = 80;
	gTileClips[TILE_RIGHT].w = TILE_WIDTH;
	gTileClips[TILE_RIGHT].h = TILE_HEIGHT;

	gTileClips[TILE_BOTTOMRIGHT].x = 240;
	gTileClips[TILE_BOTTOMRIGHT].y = 160;
	gTileClips[TILE_BOTTOMRIGHT].w = TILE_WIDTH;
	gTileClips[TILE_BOTTOMRIGHT].h = TILE_HEIGHT;

	fclose(fp);

	return 0;
}

short touchesWall(SDL_Rect *box, Tile* tiles[])
{
	int i;
	for(i = 0; i < TOTAL_TILES; ++i)
		if((Tile_getType(tiles[i]) >= TILE_CENTER)
				&& (Tile_getType(tiles[i]) <= TILE_TOPLEFT))
			if(checkCollision(box, Tile_getBox(tiles[i])))
				return 1;
	return 0;
}

int main(int argc, char* args[])
{
	int i;
	Dot dot;
	SDL_Event e;
	SDL_Rect camera = { 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT };

	if(init())
		goto equit;

	Tile* tileSet[TOTAL_TILES];

	if(loadMedia(tileSet))
		goto equit;

	Dot_init(&dot);

	while(1)
	{
		while(SDL_PollEvent(&e) != 0) {
			if(e.type == SDL_QUIT)
				goto equit;

			if(e.type == SDL_KEYDOWN)
				if(e.key.keysym.sym == SDLK_q)
					goto equit;

			Dot_handleEvent(&dot, &e);
		}

		Dot_move(&dot, &tileSet[0]);
		Dot_setCamera(&dot, &camera);

		SDL_SetRenderDrawColor(gRenderer, 0xFF, 0xFF, 0xFF, 0xFF);
		SDL_RenderClear(gRenderer);

		for(i = 0; i < TOTAL_TILES; ++i)
			Tile_render(tileSet[i], &camera);

		Dot_render(&dot, &camera);

		SDL_RenderPresent(gRenderer);
	}
equit:
	close_all(&tileSet[0]);

	return 0;
}

