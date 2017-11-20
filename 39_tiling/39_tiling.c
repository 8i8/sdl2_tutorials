/*This source code copyrighted by Lazy Foo' Productions (2004-2015)
and may not be redistributed without written permission.*/

//Using SDL, SDL_image, standard IO, strings, and file streams
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <stdio.h>

//Screen dimension constants
const int SCREEN_WIDTH = 640;
const int SCREEN_HEIGHT = 480;

//The dimensions of the level
const int LEVEL_WIDTH = 1280;
const int LEVEL_HEIGHT = 960;

//Tile constants
#define TILE_WIDTH		80
#define TILE_HEIGHT		80
#define TOTAL_TILES		192
#define TOTAL_TILE_SPRITES	12

//The dimensions of the dot
static const int DOT_WIDTH = 20;
static const int DOT_HEIGHT = 20;

//Maximum axis velocity of the dot
static const int DOT_VEL = 10;

//The different tile sprites
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

SDL_Window* gWindow = NULL;		//The window we'll be rendering to
SDL_Renderer* gRenderer = NULL;		//The window renderer
LTexture gDotTexture;			//Scene textures
LTexture gTileTexture;
SDL_Rect gTileClips[TOTAL_TILE_SPRITES];

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
	//Get rid of preexisting texture
	LTexture_free(lt);

	//The final texture
	SDL_Texture* newTexture = NULL;

	//Load image at specified path
	SDL_Surface* loadedSurface = IMG_Load(path);
	if(loadedSurface == NULL) {
		printf("Unable to load image %s! SDL_image Error: %s\n",
				path, IMG_GetError());
		return 1;
	}

	//Color key image
	SDL_SetColorKey(
			loadedSurface,
			SDL_TRUE,
			SDL_MapRGB(loadedSurface->format, 0, 0xFF, 0xFF));

	//Create texture from surface pixels
	newTexture = SDL_CreateTextureFromSurface(gRenderer, loadedSurface);
	if(newTexture == NULL) {
		printf("Unable to create texture from %s! SDL Error: %s\n",
				path, SDL_GetError());
		return 1;
	}

	//Get image dimensions
	lt->mWidth = loadedSurface->w;
	lt->mHeight = loadedSurface->h;

	//Get rid of old loaded surface
	SDL_FreeSurface(loadedSurface);

	//Return success
	lt->mTexture = newTexture;
	return 0;
}

#ifdef _SDL_TTF_H
short LTexture_loadFromRenderedText(
					LTexture *lt,
					char *textureText,
					SDL_Color textColor)
{
	//Get rid of preexisting texture
	LTexture_free(lt);

	//Render text surface
	SDL_Surface* textSurface = TTF_RenderText_Solid(
							gFont,
							textureText.c_str(),
							textColor);
	if(textSurface == NULL) {
		printf("Unable to render text surface! SDL_ttf Error: %s\n",
				TTF_GetError());
		return -1;
	}

	//Create texture from surface pixels
	mTexture = SDL_CreateTextureFromSurface(gRenderer, textSurface);
	if(mTexture == NULL) {
		printf("Unable to create texture from rendered text! SDL Error: %s\n",
				SDL_GetError());
		return -1;
	}

	//Get image dimensions
	mWidth = textSurface->w;
	mHeight = textSurface->h;

	//Get rid of old surface
	SDL_FreeSurface(textSurface);
	
	//Return success
	return 0;
}
#endif

void LTexture_setColor(LTexture *lt, Uint8 red, Uint8 green, Uint8 blue)
{
	//Modulate texture rgb
	SDL_SetTextureColorMod(lt->mTexture, red, green, blue);
}

void LTexture_setBlendMode(LTexture *lt, SDL_BlendMode blending)
{
	//Set blending function
	SDL_SetTextureBlendMode(lt->mTexture, blending);
}
		
void LTexture_setAlpha(LTexture *lt, Uint8 alpha)
{
	//Modulate texture alpha
	SDL_SetTextureAlphaMod(lt->mTexture, alpha);
}

void LTexture_render(
			LTexture *lt, 
			int x, int y,
			SDL_Rect* clip)
{
	//Set rendering space and render to screen
	SDL_Rect renderQuad = { x, y, lt->mWidth, lt->mHeight };

	//Set clip rendering dimensions
	if(clip != NULL) {
		renderQuad.w = clip->w;
		renderQuad.h = clip->h;
	}

	//Render to screen
	SDL_RenderCopy(gRenderer, lt->mTexture, clip, &renderQuad);
}

int LTexture_getWidth(LTexture *lt)
{
	return lt->mWidth;
}

int LTexture_getHeight(LTexture *lt)
{
	return lt->mHeight;
}

void Tile_init(Tile *t, int x, int y, int tileType)
{
	//Get the offsets
	t->mBox.x = x;
	t->mBox.y = y;

	//Set the collision box
	t->mBox.w = TILE_WIDTH;
	t->mBox.h = TILE_HEIGHT;

	//Get the tile type
	t->mType = tileType;
}

void Tile_render(Tile *t, SDL_Rect *camera)
{
	//If the tile is on screen
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
	//Initialize the collision box
	d->mBox.x = 0;
	d->mBox.y = 0;
	d->mBox.w = DOT_WIDTH;
	d->mBox.h = DOT_HEIGHT;

	//Initialize the velocity
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
	//Move the dot left or right
	d->mBox.x += d->mVelX;

	//If the dot went too far to the left or right or touched a wall
	if((d->mBox.x < 0) || (d->mBox.x + DOT_WIDTH > LEVEL_WIDTH)
			|| touchesWall(&d->mBox, tiles))
		d->mBox.x -= d->mVelX;

	//Move the dot up or down
	d->mBox.y += d->mVelY;

	//If the dot went too far up or down or touched a wall
	if((d->mBox.y < 0) || (d->mBox.y + DOT_HEIGHT > LEVEL_HEIGHT)
			|| touchesWall(&d->mBox, tiles))
		d->mBox.y -= d->mVelY;
}

void Dot_setCamera(Dot *d, SDL_Rect *camera)
{
	//Center the camera over the dot
	camera->x = (d->mBox.x + DOT_WIDTH / 2) - SCREEN_WIDTH / 2;
	camera->y = (d->mBox.y + DOT_HEIGHT / 2) - SCREEN_HEIGHT / 2;

	//Keep the camera in bounds
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
	//Show the dot
	LTexture_render(
			&gDotTexture,
			d->mBox.x - camera->x,
			d->mBox.y - camera->y,
			NULL);
}

short init()
{
	//Initialize SDL
	if(SDL_Init(SDL_INIT_VIDEO) < 0) {
		printf("SDL could not initialize! SDL Error: %s\n",
				SDL_GetError());
		return -1;
	}

	//Set texture filtering to linear
	if(SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "1") == 0)
		printf("Warning: Linear texture filtering not enabled!");

	//Create window
	gWindow = SDL_CreateWindow(
					"SDL Tutorial",
					SDL_WINDOWPOS_UNDEFINED,
					SDL_WINDOWPOS_UNDEFINED,
					SCREEN_WIDTH,
					SCREEN_HEIGHT,
					SDL_WINDOW_SHOWN);
	if(gWindow == NULL) {
		printf("Window could not be created! SDL Error: %s\n",
				SDL_GetError());
		return -1;
	}

	//Create renderer for window
	gRenderer = SDL_CreateRenderer(
					gWindow,
					-1,
					SDL_RENDERER_ACCELERATED
					| SDL_RENDERER_PRESENTVSYNC);
	if(gRenderer == NULL) {
		printf("Renderer could not be created! SDL Error: %s\n",
				SDL_GetError());
		return -1;
	}

	//Initialize renderer color
	SDL_SetRenderDrawColor(gRenderer, 0xFF, 0xFF, 0xFF, 0xFF);

	//Initialize PNG loading
	if(!(IMG_Init(IMG_INIT_PNG) & IMG_INIT_PNG)) {
		printf("SDL_image could not initialize! SDL_image Error: %s\n",
				IMG_GetError());
		return -1;
	}

	return 0;
}

short loadMedia(Tile* tiles[])
{
	//Load dot texture
	if(LTexture_loadFromFile(&gDotTexture, "dot.bmp")) {
		printf("Failed to load dot texture!\n");
		return -1;
	}

	//Load tile texture
	if(LTexture_loadFromFile(&gTileTexture, "tiles.png")) {
		printf("Failed to load tile set texture!\n");
		return -1;
	}

	//Load tile map
	if(setTiles(tiles)) {
		printf("Failed to load tile set!\n");
		return -1;
	}

	return 0;
}

void close_all(Tile* tiles[])
{
	int i;
	//Deallocate tiles
	for(i = 0; i < TOTAL_TILES; ++i)
		if(tiles[i] == NULL) {
			free(tiles[i]);
			tiles[i] = NULL;
		}

	//Free loaded images
	LTexture_free(&gDotTexture);
	LTexture_free(&gTileTexture);

	//Destroy window	
	SDL_DestroyRenderer(gRenderer);
	SDL_DestroyWindow(gWindow);
	gWindow = NULL;
	gRenderer = NULL;

	//Quit SDL subsystems
	IMG_Quit();
	SDL_Quit();
}

short checkCollision(SDL_Rect *a, SDL_Rect *b)
{
	//The sides of the rectangles
	int leftA, leftB;
	int rightA, rightB;
	int topA, topB;
	int bottomA, bottomB;

	//Calculate the sides of rect A
	leftA = a->x;
	rightA = a->x + a->w;
	topA = a->y;
	bottomA = a->y + a->h;

	//Calculate the sides of rect B
	leftB = b->x;
	rightB = b->x + b->w;
	topB = b->y;
	bottomB = b->y + b->h;

	//If any of the sides from A are outside of B
	if(bottomA <= topB)
		return 0;
	if(topA >= bottomB)
		return 0;
	if(rightA <= leftB)
		return 0;
	if(leftA >= rightB)
		return 0;

	//If none of the sides from A are outside B
	return 1;
}

short setTiles(Tile* tiles[])
{
	//The tile offsets
	int x = 0, y = 0, i;
	int tile = -1;
	FILE *fp;

	//Open the map
	fp = fopen("lazy.map", "r");

	if(fp == NULL) {
		printf("Unable to load map file!\n");
		return -1;
	}

	for(i = 0; i < TOTAL_TILES; ++i)
	{
		//Determines what kind of tile will be made
		int *tileType;
		tileType = &tile;

		if(fscanf(fp, "%d", tileType) < 0) {
			printf("Error loading map: Unexpected end of file!\n");
			return -1;
		}

		if((*tileType >= 0) && (*tileType < TOTAL_TILE_SPRITES) == 0) {
			printf("Error loading map: Invalid tile type at %d!\n", i);
			return -1;
		}
		tiles[i] = malloc(sizeof(Tile));
		Tile_init(tiles[i], x, y, *tileType);
		
		//Move to next tile spot
		x += TILE_WIDTH;

		//If we've gone too far
		if(x >= LEVEL_WIDTH) {
			x = 0;
			y += TILE_HEIGHT;
		}
	}
		
	//Clip the sprite sheet
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
	//Go through the tiles
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

	//Start up SDL and create window
	if(init()) {
		printf("Failed to initialize!\n");
		goto equit;
	}

	//The level tiles
	Tile* tileSet[TOTAL_TILES];

	//Load media
	if(loadMedia(tileSet)) {
		printf("Failed to load media!\n");
		goto equit;
	}

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

		//Move the dot
		Dot_move(&dot, &tileSet[0]);
		Dot_setCamera(&dot, &camera);

		//Clear screen
		SDL_SetRenderDrawColor(gRenderer, 0xFF, 0xFF, 0xFF, 0xFF);
		SDL_RenderClear(gRenderer);

		//Render level
		for(i = 0; i < TOTAL_TILES; ++i)
			Tile_render(tileSet[i], &camera);

		//Render dot
		Dot_render(&dot, &camera);

		//Update screen
		SDL_RenderPresent(gRenderer);
	}
equit:
	//Free resources and close SDL
	close_all(&tileSet[0]);

	return 0;
}
