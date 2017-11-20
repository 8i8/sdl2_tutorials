/*This source code copyrighted by Lazy Foo' Productions (2004-2015)
and may not be redistributed without written permission.*/

//Using SDL, SDL_image, SDL_ttf, standard IO, math, strings, and string streams
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <stdio.h>
#include <string>

//Screen dimension constants
const int SCREEN_WIDTH = 640;
const int SCREEN_HEIGHT = 480;

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

		//Pixel manipulators
		bool lockTexture();
		bool unlockTexture();
		void* getPixels();
		int getPitch();
		Uint32 getPixel32(unsigned int x, unsigned int y);

	private:
		//The actual hardware texture
		SDL_Texture* mTexture;
		void* mPixels;
		int mPitch;

		//Image dimensions
		int mWidth;
		int mHeight;
};

//Our bitmap font
class LBitmapFont
{
	public:
		//The default constructor
		LBitmapFont();

		//Generates the font
		bool buildFont(LTexture *bitmap);

		//Shows the text
		short renderText(int x, int y, std::string text);

	private:
		//The font texture
		LTexture* mBitmap;

		//The individual characters in the surface
		SDL_Rect mChars[ 256 ];

		//Spacing Variables
		int mNewLine, mSpace;
};

SDL_Window* gWindow = NULL;		//The window we'll be rendering to
SDL_Renderer* gRenderer = NULL;		//The window renderer
LTexture gBitmapTexture;		//Scene textures
LBitmapFont gBitmapFont;

LTexture::LTexture()
{
	//Initialize
	mTexture = NULL;
	mWidth = 0;
	mHeight = 0;
	mPixels = NULL;
	mPitch = 0;
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
	}

	//Convert surface to display format
	SDL_Surface* formattedSurface = SDL_ConvertSurfaceFormat(
							loadedSurface,
							SDL_PIXELFORMAT_RGBA8888,
							SDL_SWSURFACE);
	if(formattedSurface == NULL) {
		printf("Unable to convert loaded surface to display format! %s\n",
				SDL_GetError());
		return -1;
	}

	//Create blank streamable texture
	newTexture = SDL_CreateTexture(
					gRenderer,
					SDL_PIXELFORMAT_RGBA8888,
					SDL_TEXTUREACCESS_STREAMING,
					formattedSurface->w,
					formattedSurface->h);
	if(newTexture == NULL) {
		printf("Unable to create blank texture! SDL Error: %s\n",
				SDL_GetError());
		return -1;
	}

	//Enable blending on texture
	SDL_SetTextureBlendMode(newTexture, SDL_BLENDMODE_BLEND);

	//Lock texture for manipulation
	SDL_LockTexture(
				newTexture,
				&formattedSurface->clip_rect,
				&mPixels,
				&mPitch);

	//Copy loaded/formatted surface pixels
	memcpy(
			mPixels,
			formattedSurface->pixels,
			formattedSurface->pitch * formattedSurface->h);

	//Get image dimensions
	mWidth = formattedSurface->w;
	mHeight = formattedSurface->h;

	//Get pixel data in editable format
	Uint32* pixels = (Uint32*)mPixels;
	int pixelCount = (mPitch / 4) * mHeight;

	//Map colors				
	Uint32 colorKey = SDL_MapRGB(formattedSurface->format, 0, 0xFF, 0xFF);
	Uint32 transparent = SDL_MapRGBA(
			formattedSurface->format, 0x00, 0xFF, 0xFF, 0x00);

	//Color key pixels
	int i;
	for(i = 0; i < pixelCount; ++i)
		if(pixels[ i ] == colorKey)
			pixels[ i ] = transparent;

	//Unlock texture to update
	SDL_UnlockTexture(newTexture);
	mPixels = NULL;

	//Get rid of old formatted surface
	SDL_FreeSurface(formattedSurface);
		
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
	if(textSurface != NULL) {
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
	if(mTexture != NULL)
	{
		SDL_DestroyTexture(mTexture);
		mTexture = NULL;
		mWidth = 0;
		mHeight = 0;
		mPixels = NULL;
		mPitch = 0;
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
	if(clip != NULL)
	{
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

bool LTexture::lockTexture()
{
	if(mPixels != NULL) {
		printf("Texture is already locked!\n");
		return -1;
	}
	if(SDL_LockTexture(mTexture, NULL, &mPixels, &mPitch) != 0) {
		printf("Unable to lock texture! %s\n", SDL_GetError());
		return -1;
	}
	return 0;
}

bool LTexture::unlockTexture()
{
	if(mPixels == NULL) {
		printf("Texture is not locked!\n");
		return -1;
	}

	SDL_UnlockTexture(mTexture);
	mPixels = NULL;
	mPitch = 0;

	return 0;
}

void* LTexture::getPixels()
{
	return mPixels;
}

int LTexture::getPitch()
{
	return mPitch;
}

Uint32 LTexture::getPixel32(unsigned int x, unsigned int y)
{
	//Convert the pixels to 32 bit
	Uint32 *pixels = (Uint32*)mPixels;

	//Get the pixel requested
	return pixels[ (y * (mPitch / 4)) + x ];
}

LBitmapFont::LBitmapFont()
{
	mBitmap = NULL;
	mNewLine = 0;
	mSpace = 0;
}

bool LBitmapFont::buildFont(LTexture* bitmap)
{
	if(bitmap->lockTexture()) {
		printf("Unable to lock bitmap font texture!\n");
		return -1;
	}

	//Set the background color
	Uint32 bgColor = bitmap->getPixel32(0, 0);

	//Set the cell dimensions
	int cellW = bitmap->getWidth() / 16;
	int cellH = bitmap->getHeight() / 16;

	//New line variables
	int top = cellH;
	int baseA = cellH;

	//The current character we're setting
	int currentChar = 0;

	//Go through the cell rows and columns
	for(int rows = 0; rows < 16; ++rows) {
		for(int cols = 0; cols < 16; ++cols)
		{
			//Set the character offset
			mChars[currentChar].x = cellW * cols;
			mChars[currentChar].y = cellH * rows;

			//Set the dimensions of the character
			mChars[currentChar].w = cellW;
			mChars[currentChar].h = cellH;

			//Find Left Side
			//Go through pixel columns
			for(int pCol = 0; pCol < cellW; ++pCol)
			{
				//Go through pixel rows
				for(int pRow = 0; pRow < cellH; ++pRow)
				{
					//Get the pixel offsets
					int pX = (cellW * cols) + pCol;
					int pY = (cellH * rows) + pRow;

					//If a non colorkey pixel is found
					if(bitmap->getPixel32(pX, pY) != bgColor)
					{
						//Set the x offset
						mChars[currentChar].x = pX;

						//Break the loops
						pCol = cellW;
						pRow = cellH;
					}
				}
			}

			//Find Right Side
			//Go through pixel columns
			for(int pColW = cellW - 1; pColW >= 0; --pColW)
			{
				//Go through pixel rows
				for(int pRowW = 0; pRowW < cellH; ++pRowW)
				{
					//Get the pixel offsets
					int pX = (cellW * cols) + pColW;
					int pY = (cellH * rows) + pRowW;

					//If a non colorkey pixel is found
					if(bitmap->getPixel32(pX, pY) != bgColor)
					{
						//Set the width
						mChars[currentChar].w = (pX - mChars[currentChar].x) + 1;

						//Break the loops
						pColW = -1;
						pRowW = cellH;
					}
				}
			}

			//Find Top
			//Go through pixel rows
			for(int pRow = 0; pRow < cellH; ++pRow)
			{
				//Go through pixel columns
				for(int pCol = 0; pCol < cellW; ++pCol)
				{
					//Get the pixel offsets
					int pX = (cellW * cols) + pCol;
					int pY = (cellH * rows) + pRow;

					//If a non colorkey pixel is found
					if(bitmap->getPixel32(pX, pY) != bgColor)
					{
						//If new top is found
						if(pRow < top)
						{
							top = pRow;
						}

						//Break the loops
						pCol = cellW;
						pRow = cellH;
					}
				}
			}

			//Find Bottom of A
			if(currentChar == 'A')
			{
				//Go through pixel rows
				for(int pRow = cellH - 1; pRow >= 0; --pRow)
				{
					//Go through pixel columns
					for(int pCol = 0; pCol < cellW; ++pCol)
					{
						//Get the pixel offsets
						int pX = (cellW * cols) + pCol;
						int pY = (cellH * rows) + pRow;

						//If a non colorkey pixel is found
						if(bitmap->getPixel32(pX, pY) != bgColor)
						{
							//Bottom of a is found
							baseA = pRow;

							//Break the loops
							pCol = cellW;
							pRow = -1;
						}
					}
				}
			}

			//Go to the next character
			++currentChar;
		}
	}

	//Calculate space
	mSpace = cellW / 2;
	//Calculate new line
	mNewLine = baseA - top;

	//Lop off excess top pixels
	for(int i = 0; i < 256; ++i) {
		mChars[ i ].y += top;
		mChars[ i ].h -= top;
	}

	bitmap->unlockTexture();
	mBitmap = bitmap;

	return 0;
}

short LBitmapFont::renderText(int x, int y, std::string text)
{
	//If the font has been built
	if(mBitmap == NULL)
		return -1;

	//Temp offsets
	int curX = x, curY = y;

	//Go through the text
	for(unsigned i = 0; i < text.length(); ++i)
	{
		//If the current character is a space
		if(text[i] == ' ')
			curX += mSpace;
		//If the current character is a newline
		else if(text[i] == '\n')
		{
			//Move down
			curY += mNewLine;

			//Move back
			curX = x;
		}
		else
		{
			//Get the ASCII value of the character
			int ascii = (unsigned char)text[i];

			//Show the character
			mBitmap->render(curX, curY, &mChars[ascii]);

			//Move over the width of the character with one pixel
			//of padding
			curX += mChars[ascii].w + 1;
		}
	}
	
	return 0;
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
	{
		printf("Warning: Linear texture filtering not enabled!");
	}

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

bool loadMedia()
{
	//Load font texture
	if(gBitmapTexture.loadFromFile("lazyfont.png")) {
		printf("Failed to load corner texture!\n");
		return -1;
	}

	//Build font from texture
	gBitmapFont.buildFont(&gBitmapTexture);

	return 0;
}

void close_all()
{
	//Free loaded images
	gBitmapTexture.free();

	//Destroy window	
	SDL_DestroyRenderer(gRenderer);
	SDL_DestroyWindow(gWindow);
	gWindow = NULL;
	gRenderer = NULL;

	//Quit SDL subsystems
	IMG_Quit();
	SDL_Quit();
}

int main(int argc, char* args[])
{
	//Start up SDL and create window
	if(init()) {
		printf("Failed to initialize!\n");
		goto equit;
	}


	//Load media
	if(loadMedia()) {
		printf("Failed to load media!\n");
		goto equit;
	}

	SDL_Event e;

	//While application is running
	while(1)
	{
		while(SDL_PollEvent(&e) != 0)
			if(e.type == SDL_QUIT)
				goto equit;

		//Clear screen
		SDL_SetRenderDrawColor(gRenderer, 0xFF, 0xFF, 0xFF, 0xFF);
		SDL_RenderClear(gRenderer);

		//Render test text
		gBitmapFont.renderText(50, 10, "Bitmap Font:\nABDCEFGHIJKLMNOPQRSTUVWXYZ");
		gBitmapFont.renderText(100, 100, "\nabcdefghijklmnopqrstuvwxyz\n0123456789");

		//Update screen
		SDL_RenderPresent(gRenderer);
	}
equit:
	//Free resources and close SDL
	close_all();

	return 0;
}
