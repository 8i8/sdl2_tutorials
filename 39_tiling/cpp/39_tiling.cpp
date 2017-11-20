/*This source code copyrighted by Lazy Foo' Productions (2004-2015)
and may not be redistributed without written permission.*/

//Using SDL, SDL_image, standard IO, strings, and file streams
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <stdio.h>
#include <string>
#include <fstream>

//Screen dimension constants
const int SCREEN_WIDTH = 640;
const int SCREEN_HEIGHT = 480;

//The dimensions of the level
const int LEVEL_WIDTH = 1280;
const int LEVEL_HEIGHT = 960;

//Tile constants
const int TILE_WIDTH = 80;
const int TILE_HEIGHT = 80;
const int TOTAL_TILES = 192;
const int TOTAL_TILE_SPRITES = 12;

//The different tile sprites
const int TILE_RED = 0;
const int TILE_GREEN = 1;
const int TILE_BLUE = 2;
const int TILE_CENTER = 3;
const int TILE_TOP = 4;
const int TILE_TOPRIGHT = 5;
const int TILE_RIGHT = 6;
const int TILE_BOTTOMRIGHT = 7;
const int TILE_BOTTOM = 8;
const int TILE_BOTTOMLEFT = 9;
const int TILE_LEFT = 10;
const int TILE_TOPLEFT = 11;

//Texture wrapper class
class LTexture
{
	public:
		//Initializes variables
		LTexture();

		//Deallocates memory
		~LTexture();

		//Loads image at specified path
		bool loadFromFile(std::string path);
		
		#ifdef _SDL_TTF_H
		//Creates image from font string
		bool loadFromRenderedText(
					std::string textureText,
					SDL_Color textColor);
		#endif

		//Deallocates texture
		void free();

		//Set color modulation
		void setColor(Uint8 red, Uint8 green, Uint8 blue);

		//Set blending
		void setBlendMode(SDL_BlendMode blending);

		//Set alpha modulation
		void setAlpha(Uint8 alpha);
		
		//Renders texture at given point
		void render(
				int x, int y,
				SDL_Rect* clip = NULL,
				double angle = 0.0,
				SDL_Point* center = NULL,
				SDL_RendererFlip flip = SDL_FLIP_NONE);

		//Gets image dimensions
		int getWidth();
		int getHeight();

	private:
		//The actual hardware texture
		SDL_Texture* mTexture;

		//Image dimensions
		int mWidth;
		int mHeight;
};

//The tile
class Tile
{
	public:
		//Initializes position and type
		Tile(int x, int y, int tileType);

		//Shows the tile
		void render(SDL_Rect& camera);

		//Get the tile type
		int getType();

		//Get the collision box
		SDL_Rect getBox();

	private:
		//The attributes of the tile
		SDL_Rect mBox;

		//The tile type
		int mType;
};

//The dot that will move around on the screen
class Dot
{
	public:
		//The dimensions of the dot
		static const int DOT_WIDTH = 20;
		static const int DOT_HEIGHT = 20;

		//Maximum axis velocity of the dot
		static const int DOT_VEL = 10;

		//Initializes the variables
		Dot();

		//Takes key presses and adjusts the dot's velocity
		void handleEvent(SDL_Event& e);

		//Moves the dot and check collision against tiles
		void move(Tile *tiles[]);

		//Centers the camera over the dot
		void setCamera(SDL_Rect& camera);

		//Shows the dot on the screen
		void render(SDL_Rect& camera);

	private:
		//Collision box of the dot
		SDL_Rect mBox;

		//The velocity of the dot
		int mVelX, mVelY;
};

//Starts up SDL and creates window
bool init();

//Loads media
bool loadMedia(Tile* tiles[]);

//Frees media and shuts down SDL
void close(Tile* tiles[]);

//Box collision detector
bool checkCollision(SDL_Rect a, SDL_Rect b);

//Checks collision box against set of tiles
bool touchesWall(SDL_Rect box, Tile* tiles[]);

//Sets tiles from tile map
bool setTiles(Tile *tiles[]);

SDL_Window* gWindow = NULL;		//The window we'll be rendering to
SDL_Renderer* gRenderer = NULL;		//The window renderer
LTexture gDotTexture;			//Scene textures
LTexture gTileTexture;
SDL_Rect gTileClips[TOTAL_TILE_SPRITES];

LTexture::LTexture()
{
	//Initialize
	mTexture = NULL;
	mWidth = 0;
	mHeight = 0;
}

LTexture::~LTexture()
{
	//Deallocate
	free();
}

bool LTexture::loadFromFile(std::string path)
{
	//Get rid of preexisting texture
	free();

	//The final texture
	SDL_Texture* newTexture = NULL;

	//Load image at specified path
	SDL_Surface* loadedSurface = IMG_Load(path.c_str());
	if(loadedSurface == NULL) {
		printf("Unable to load image %s! SDL_image Error: %s\n",
				path.c_str(), IMG_GetError());
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
				path.c_str(), SDL_GetError());
		return 1;
	}

	//Get image dimensions
	mWidth = loadedSurface->w;
	mHeight = loadedSurface->h;

	//Get rid of old loaded surface
	SDL_FreeSurface(loadedSurface);

	//Return success
	mTexture = newTexture;
	return 0;
}

#ifdef _SDL_TTF_H
bool LTexture::loadFromRenderedText(std::string textureText, SDL_Color textColor)
{
	//Get rid of preexisting texture
	free();

	//Render text surface
	SDL_Surface* textSurface = TTF_RenderText_Solid(
							gFont,
							textureText.c_str(),
							textColor);
	if(textSurface == NULL)
	{
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
	return mTexture != NULL;
}
#endif

void LTexture::free()
{
	//Free texture if it exists
	if(mTexture != NULL) {
		SDL_DestroyTexture(mTexture);
		mTexture = NULL;
		mWidth = 0;
		mHeight = 0;
	}
}

void LTexture::setColor(Uint8 red, Uint8 green, Uint8 blue)
{
	//Modulate texture rgb
	SDL_SetTextureColorMod(mTexture, red, green, blue);
}

void LTexture::setBlendMode(SDL_BlendMode blending)
{
	//Set blending function
	SDL_SetTextureBlendMode(mTexture, blending);
}
		
void LTexture::setAlpha(Uint8 alpha)
{
	//Modulate texture alpha
	SDL_SetTextureAlphaMod(mTexture, alpha);
}

void LTexture::render(
			int x, int y,
			SDL_Rect* clip,
			double angle,
			SDL_Point* center,
			SDL_RendererFlip flip)
{
	//Set rendering space and render to screen
	SDL_Rect renderQuad = { x, y, mWidth, mHeight };

	//Set clip rendering dimensions
	if(clip != NULL) {
		renderQuad.w = clip->w;
		renderQuad.h = clip->h;
	}

	//Render to screen
	SDL_RenderCopyEx(gRenderer, mTexture, clip,
				&renderQuad, angle, center, flip);
}

int LTexture::getWidth()
{
	return mWidth;
}

int LTexture::getHeight()
{
	return mHeight;
}

Tile::Tile(int x, int y, int tileType)
{
	//Get the offsets
	mBox.x = x;
	mBox.y = y;

	//Set the collision box
	mBox.w = TILE_WIDTH;
	mBox.h = TILE_HEIGHT;

	//Get the tile type
	mType = tileType;
}

void Tile::render(SDL_Rect& camera)
{
	//If the tile is on screen
	if(checkCollision(camera, mBox))
		gTileTexture.render(
					mBox.x - camera.x,
					mBox.y - camera.y,
					&gTileClips[mType]);
}

int Tile::getType()
{
	return mType;
}

SDL_Rect Tile::getBox()
{
	return mBox;
}

Dot::Dot()
{
	//Initialize the collision box
	mBox.x = 0;
	mBox.y = 0;
	mBox.w = DOT_WIDTH;
	mBox.h = DOT_HEIGHT;

	//Initialize the velocity
	mVelX = 0;
	mVelY = 0;
}

void Dot::handleEvent(SDL_Event& e)
{
	//If a key was pressed
	if(e.type == SDL_KEYDOWN && e.key.repeat == 0)
	{
		//Adjust the velocity
		switch(e.key.keysym.sym)
		{
			case SDLK_UP: mVelY -= DOT_VEL; break;
			case SDLK_DOWN: mVelY += DOT_VEL; break;
			case SDLK_LEFT: mVelX -= DOT_VEL; break;
			case SDLK_RIGHT: mVelX += DOT_VEL; break;
		}
	}
	//If a key was released
	else if(e.type == SDL_KEYUP && e.key.repeat == 0)
	{
		//Adjust the velocity
		switch(e.key.keysym.sym)
		{
			case SDLK_UP: mVelY += DOT_VEL; break;
			case SDLK_DOWN: mVelY -= DOT_VEL; break;
			case SDLK_LEFT: mVelX += DOT_VEL; break;
			case SDLK_RIGHT: mVelX -= DOT_VEL; break;
		}
	}
}

void Dot::move(Tile *tiles[])
{
	//Move the dot left or right
	mBox.x += mVelX;

	//If the dot went too far to the left or right or touched a wall
	if((mBox.x < 0) || (mBox.x + DOT_WIDTH > LEVEL_WIDTH)
			|| touchesWall(mBox, tiles))
		mBox.x -= mVelX;

	//Move the dot up or down
	mBox.y += mVelY;

	//If the dot went too far up or down or touched a wall
	if((mBox.y < 0) || (mBox.y + DOT_HEIGHT > LEVEL_HEIGHT)
			|| touchesWall(mBox, tiles))
		mBox.y -= mVelY;
}

void Dot::setCamera(SDL_Rect& camera)
{
	//Center the camera over the dot
	camera.x = (mBox.x + DOT_WIDTH / 2) - SCREEN_WIDTH / 2;
	camera.y = (mBox.y + DOT_HEIGHT / 2) - SCREEN_HEIGHT / 2;

	//Keep the camera in bounds
	if(camera.x < 0)
		camera.x = 0;
	if(camera.y < 0)
		camera.y = 0;
	if(camera.x > LEVEL_WIDTH - camera.w)
		camera.x = LEVEL_WIDTH - camera.w;
	if(camera.y > LEVEL_HEIGHT - camera.h)
		camera.y = LEVEL_HEIGHT - camera.h;
}

void Dot::render(SDL_Rect& camera)
{
	//Show the dot
	gDotTexture.render(mBox.x - camera.x, mBox.y - camera.y);
}

bool init()
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

bool loadMedia(Tile* tiles[])
{
	//Load dot texture
	if(gDotTexture.loadFromFile("dot.bmp")) {
		printf("Failed to load dot texture!\n");
		return -1;
	}

	//Load tile texture
	if(gTileTexture.loadFromFile("tiles.png")) {
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

void close(Tile* tiles[])
{
	int i;
	//Deallocate tiles
	for(i = 0; i < TOTAL_TILES; ++i)
		 if(tiles[ i ] == NULL) {
			delete tiles[ i ];
			tiles[ i ] = NULL;
		 }

	//Free loaded images
	gDotTexture.free();
	gTileTexture.free();

	//Destroy window	
	SDL_DestroyRenderer(gRenderer);
	SDL_DestroyWindow(gWindow);
	gWindow = NULL;
	gRenderer = NULL;

	//Quit SDL subsystems
	IMG_Quit();
	SDL_Quit();
}

bool checkCollision(SDL_Rect a, SDL_Rect b)
{
	//The sides of the rectangles
	int leftA, leftB;
	int rightA, rightB;
	int topA, topB;
	int bottomA, bottomB;

	//Calculate the sides of rect A
	leftA = a.x;
	rightA = a.x + a.w;
	topA = a.y;
	bottomA = a.y + a.h;

	//Calculate the sides of rect B
	leftB = b.x;
	rightB = b.x + b.w;
	topB = b.y;
	bottomB = b.y + b.h;

	//If any of the sides from A are outside of B
	if(bottomA <= topB)
		return false;
	if(topA >= bottomB)
		return false;
	if(rightA <= leftB)
		return false;
	if(leftA >= rightB)
		return false;

	//If none of the sides from A are outside B
	return true;
}

bool setTiles(Tile* tiles[])
{
	//The tile offsets
	int x = 0, y = 0, i;

	//Open the map
	std::ifstream map("lazy.map");

	//If the map couldn't be loaded
	if(map == NULL) {
		printf("Unable to load map file!\n");
		return -1;
	}

	//Initialize the tiles
	for(i = 0; i < TOTAL_TILES; ++i)
	{
		//Determines what kind of tile will be made
		int tileType = -1;

		//Read tile from map file
		map >> tileType;

		//If the was a problem in reading the map
		if(map.fail()) {
			//Stop loading map
			printf("Error loading map: Unexpected end of file!\n");
			return -1;
		}

		//If the number is not a valid tile number
		if((tileType >= 0) && (tileType < TOTAL_TILE_SPRITES) == 0) {
			printf("Error loading map: Invalid tile type at %d!\n", i);
			return -1;
		}
		// Else get the tile
		tiles[ i ] = new Tile(x, y, tileType);
		
		//If we don't recognize the tile type
		//Move to next tile spot
		x += TILE_WIDTH;

		//If we've gone too far
		if(x >= LEVEL_WIDTH) {
			x = 0;
			y += TILE_HEIGHT;
		}
	}
		
	//Clip the sprite sheet
	gTileClips[ TILE_RED ].x = 0;
	gTileClips[ TILE_RED ].y = 0;
	gTileClips[ TILE_RED ].w = TILE_WIDTH;
	gTileClips[ TILE_RED ].h = TILE_HEIGHT;

	gTileClips[ TILE_GREEN ].x = 0;
	gTileClips[ TILE_GREEN ].y = 80;
	gTileClips[ TILE_GREEN ].w = TILE_WIDTH;
	gTileClips[ TILE_GREEN ].h = TILE_HEIGHT;

	gTileClips[ TILE_BLUE ].x = 0;
	gTileClips[ TILE_BLUE ].y = 160;
	gTileClips[ TILE_BLUE ].w = TILE_WIDTH;
	gTileClips[ TILE_BLUE ].h = TILE_HEIGHT;

	gTileClips[ TILE_TOPLEFT ].x = 80;
	gTileClips[ TILE_TOPLEFT ].y = 0;
	gTileClips[ TILE_TOPLEFT ].w = TILE_WIDTH;
	gTileClips[ TILE_TOPLEFT ].h = TILE_HEIGHT;

	gTileClips[ TILE_LEFT ].x = 80;
	gTileClips[ TILE_LEFT ].y = 80;
	gTileClips[ TILE_LEFT ].w = TILE_WIDTH;
	gTileClips[ TILE_LEFT ].h = TILE_HEIGHT;

	gTileClips[ TILE_BOTTOMLEFT ].x = 80;
	gTileClips[ TILE_BOTTOMLEFT ].y = 160;
	gTileClips[ TILE_BOTTOMLEFT ].w = TILE_WIDTH;
	gTileClips[ TILE_BOTTOMLEFT ].h = TILE_HEIGHT;

	gTileClips[ TILE_TOP ].x = 160;
	gTileClips[ TILE_TOP ].y = 0;
	gTileClips[ TILE_TOP ].w = TILE_WIDTH;
	gTileClips[ TILE_TOP ].h = TILE_HEIGHT;

	gTileClips[ TILE_CENTER ].x = 160;
	gTileClips[ TILE_CENTER ].y = 80;
	gTileClips[ TILE_CENTER ].w = TILE_WIDTH;
	gTileClips[ TILE_CENTER ].h = TILE_HEIGHT;

	gTileClips[ TILE_BOTTOM ].x = 160;
	gTileClips[ TILE_BOTTOM ].y = 160;
	gTileClips[ TILE_BOTTOM ].w = TILE_WIDTH;
	gTileClips[ TILE_BOTTOM ].h = TILE_HEIGHT;

	gTileClips[ TILE_TOPRIGHT ].x = 240;
	gTileClips[ TILE_TOPRIGHT ].y = 0;
	gTileClips[ TILE_TOPRIGHT ].w = TILE_WIDTH;
	gTileClips[ TILE_TOPRIGHT ].h = TILE_HEIGHT;

	gTileClips[ TILE_RIGHT ].x = 240;
	gTileClips[ TILE_RIGHT ].y = 80;
	gTileClips[ TILE_RIGHT ].w = TILE_WIDTH;
	gTileClips[ TILE_RIGHT ].h = TILE_HEIGHT;

	gTileClips[ TILE_BOTTOMRIGHT ].x = 240;
	gTileClips[ TILE_BOTTOMRIGHT ].y = 160;
	gTileClips[ TILE_BOTTOMRIGHT ].w = TILE_WIDTH;
	gTileClips[ TILE_BOTTOMRIGHT ].h = TILE_HEIGHT;

	//Close the file
	map.close();

	//If the map was loaded fine
	return 0;
}

bool touchesWall(SDL_Rect box, Tile* tiles[])
{
	int i;
	//Go through the tiles
	for(i = 0; i < TOTAL_TILES; ++i)
		if((tiles[i]->getType() >= TILE_CENTER)
				&& (tiles[i]->getType() <= TILE_TOPLEFT))
			if(checkCollision(box, tiles[ i ]->getBox()))
				return true;
	return false;
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

	while(1)
	{
		while(SDL_PollEvent(&e) != 0) {
			if(e.type == SDL_QUIT)
				goto equit;

			dot.handleEvent(e);
		}

		//Move the dot
		dot.move(tileSet);
		dot.setCamera(camera);

		//Clear screen
		SDL_SetRenderDrawColor(gRenderer, 0xFF, 0xFF, 0xFF, 0xFF);
		SDL_RenderClear(gRenderer);

		//Render level
		for(i = 0; i < TOTAL_TILES; ++i)
			tileSet[i]->render(camera);

		//Render dot
		dot.render(camera);

		//Update screen
		SDL_RenderPresent(gRenderer);
	}
equit:
	//Free resources and close SDL
	close(tileSet);

	return 0;
}
