/*This source code copyrighted by Lazy Foo' Productions (2004-2015)
and may not be redistributed without written permission.*/

/*
 *  TODO find a way to simplifty/shorten the function that reads the char for
 *  the font
 */

//Using SDL, SDL_image, SDL_ttf, standard IO, math, strings, and string streams
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <stdio.h>

//Screen dimension constants
const int SCREEN_WIDTH = 640;
const int SCREEN_HEIGHT = 480;

typedef struct {
	SDL_Texture* mTexture;
	void* mPixels;
	int mPitch;
	int mWidth;
	int mHeight;
} LTexture;

typedef struct {
	LTexture* mBitmap;
	SDL_Rect mChars[256];
	int mNewLine, mSpace;
} LBitmapFont;

SDL_Window* gWindow = NULL;		//The window we'll be rendering to
SDL_Renderer* gRenderer = NULL;		//The window renderer
LTexture gBitmapTexture;		//Scene textures
LBitmapFont gBitmapFont;

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
		return -1;
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
				&lt->mPixels,
				&lt->mPitch);

	//Copy loaded/formatted surface pixels
	memcpy(
			lt->mPixels,
			formattedSurface->pixels,
			formattedSurface->pitch * formattedSurface->h);

	//Get image dimensions
	lt->mWidth = formattedSurface->w;
	lt->mHeight = formattedSurface->h;

	//Get pixel data in editable format
	Uint32* pixels = (Uint32*)lt->mPixels;
	int pixelCount = (lt->mPitch / 4) * lt->mHeight;

	//Map colors				
	Uint32 colorKey = SDL_MapRGB(formattedSurface->format, 0, 0xFF, 0xFF);
	Uint32 transparent = SDL_MapRGBA(
			formattedSurface->format, 0x00, 0xFF, 0xFF, 0x00);

	//Color key pixels
	int i;
	for(i = 0; i < pixelCount; ++i)
		if(pixels[i] == colorKey)
			pixels[i] = transparent;

	//Unlock texture to update
	SDL_UnlockTexture(newTexture);
	lt->mPixels = NULL;

	//Get rid of old formatted surface
	SDL_FreeSurface(formattedSurface);
		
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
							textureText,
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
	lt->mWidth = textSurface->w;
	lt->mHeight = textSurface->h;

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
			SDL_Rect* clip,
			double angle,
			SDL_Point* center,
			SDL_RendererFlip flip)
{
	//Set rendering space and render to screen
	SDL_Rect renderQuad = { x, y, lt->mWidth, lt->mHeight };

	//Set clip rendering dimensions
	if(clip != NULL) {
		renderQuad.w = clip->w;
		renderQuad.h = clip->h;
	}

	//Render to screen
	SDL_RenderCopyEx(gRenderer, lt->mTexture, clip,
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

short LTexture_lockTexture(LTexture *lt)
{
	if(lt->mPixels != NULL) {
		printf("Texture is already locked!\n");
		return -1;
	}

	if(SDL_LockTexture(lt->mTexture, NULL, &lt->mPixels, &lt->mPitch) != 0) {
		printf("Unable to lock texture! %s\n", SDL_GetError());
		return -1;
	}
	return 0;
}

short LTexture_unlockTexture(LTexture *lt)
{
	if(lt->mPixels == NULL) {
		printf("Texture is not locked!\n");
		return -1;
	}

	SDL_UnlockTexture(lt->mTexture);
	lt->mPixels = NULL;
	lt->mPitch = 0;

	return 0;
}

void* LTexture_getPixels(LTexture *lt)
{
	return lt->mPixels;
}

int LTexture_getPitch(LTexture *lt)
{
	return lt->mPitch;
}

Uint32 LTexture_getPixel32(LTexture *lt, unsigned int x, unsigned int y)
{
	//Convert the pixels to 32 bit
	Uint32 *pixels = (Uint32*)lt->mPixels;

	//Get the pixel requested
	return pixels[(y * (lt->mPitch / 4)) + x];
}

void LBitmapFont_init(LBitmapFont *lb)
{
	lb->mBitmap = NULL;
	lb->mNewLine = 0;
	lb->mSpace = 0;
}

//void innerfunc(int *A, int B, int *C, int D, int *E, int F)
//{
//	*A = B; *C = D; *E = F;
//}
//
//void function(
//		LTexture *bitmap, Uint32 bgColor,
//		int cols, int rows, int A, int B, int C, int D)
//{
//	int pX, pY;
//
//	for( A; A < B; ++A) {
//		for( C; C < D; ++C) {
//
//			//Get the pixel offsets
//			pX = (B * cols) + A;
//			pY = (D * rows) + C;
//
//			//If a non colorkey pixel is found
//			if(LTexture_getPixel32(bitmap, pX, pY) != bgColor)
//			{
//				//Set the x offset
//				lb->mChars[currentChar].x = pX;
//
//				//Break the loops
//				pCol = cellW;
//				pRow = cellH;
//			}
//		}
//	}
//}

short LBitmapFont_buildFont(LBitmapFont *lb, LTexture* bitmap)
{
	if(LTexture_lockTexture(bitmap)) {
		printf("Unable to lock bitmap font texture!\n");
		return -1;
	}

	//Set the background color
	Uint32 bgColor = LTexture_getPixel32(bitmap, 0, 0);

	//Set the cell dimensions
	int cellW = LTexture_getWidth(bitmap) / 16;
	int cellH = LTexture_getHeight(bitmap) / 16;

	//New line variables
	int top = cellH;
	int baseA = cellH;

	//The current character we're setting
	int currentChar = 0;
	int rows, cols;
	int pCol, pRow;
	int pX, pY;

	//Go through the cell rows and columns
	for(rows = 0; rows < 16; ++rows) {
		for(cols = 0; cols < 16; ++cols)
		{
			//Set the character offset
			lb->mChars[currentChar].x = cellW * cols;
			lb->mChars[currentChar].y = cellH * rows;

			//Set the dimensions of the character
			lb->mChars[currentChar].w = cellW;
			lb->mChars[currentChar].h = cellH;

			//Find Left Side
			//Go through pixel columns
			for(pCol = 0; pCol < cellW; ++pCol)
			{
				//Go through pixel rows
				for(pRow = 0; pRow < cellH; ++pRow)
				{
					//Get the pixel offsets
					pX = (cellW * cols) + pCol;
					pY = (cellH * rows) + pRow;

					//If a non colorkey pixel is found
					if(LTexture_getPixel32(bitmap, pX, pY) != bgColor)
					{
						//Set the x offset
						lb->mChars[currentChar].x = pX;

						//Break the loops
						pCol = cellW;
						pRow = cellH;
					}
				}
			}

			//Find Right Side
			//Go through pixel columns
			for(pCol = cellW - 1; pCol >= 0; --pCol)
			{
				//Go through pixel rows
				for(pRow = 0; pRow < cellH; ++pRow)
				{
					//Get the pixel offsets
					pX = (cellW * cols) + pCol;
					pY = (cellH * rows) + pRow;

					//If a non colorkey pixel is found
					if(LTexture_getPixel32(bitmap, pX, pY) != bgColor)
					{
						//Set the width
						lb->mChars[currentChar].w = (pX - lb->mChars[currentChar].x) + 1;

						//Break the loops
						pCol = -1;
						pRow = cellH;
					}
				}
			}

			//Find Top
			//Go through pixel rows
			for(pRow = 0; pRow < cellH; ++pRow)
			{
				//Go through pixel columns
				for(pCol = 0; pCol < cellW; ++pCol)
				{
					//Get the pixel offsets
					pX = (cellW * cols) + pCol;
					pY = (cellH * rows) + pRow;

					//If a non colorkey pixel is found
					if(LTexture_getPixel32(bitmap, pX, pY) != bgColor)
					{
						//If new top is found
						if(pRow < top)
							top = pRow;

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
				for(pRow = cellH - 1; pRow >= 0; --pRow)
				{
					//Go through pixel columns
					for(pCol = 0; pCol < cellW; ++pCol)
					{
						//Get the pixel offsets
						pX = (cellW * cols) + pCol;
						pY = (cellH * rows) + pRow;

						//If a non colorkey pixel is found
						if(LTexture_getPixel32(bitmap, pX, pY) != bgColor)
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
	lb->mSpace = cellW / 2;
	//Calculate new line
	lb->mNewLine = baseA - top;

	//Lop off excess top pixels
	for(int i = 0; i < 256; ++i) {
		lb->mChars[ i ].y += top;
		lb->mChars[ i ].h -= top;
	}

	LTexture_unlockTexture(bitmap);
	lb->mBitmap = bitmap;

	return 0;
}

short LBitmapFont_renderText(LBitmapFont *lb, int x, int y, char *text)
{
	//If the font has been built
	if(lb->mBitmap == NULL)
		return -1;

	//Temp offsets
	int curX = x, curY = y;

	//Go through the text
	for(unsigned i = 0; i < strlen(text); ++i)
	{
		//If the current character is a space
		if(text[i] == ' ')
			curX += lb->mSpace;
		//If the current character is a newline
		else if(text[i] == '\n')
		{
			//Move down
			curY += lb->mNewLine;

			//Move back
			curX = x;
		}
		else
		{
			//Get the ASCII value of the character
			int ascii = (unsigned char)text[i];

			//Show the character
			LTexture_render(
					lb->mBitmap,
					curX, curY,
					&lb->mChars[ascii],
					0.0, NULL, 0);

			//Move over the width of the character with one pixel
			//of padding
			curX += lb->mChars[ascii].w + 1;
		}
	}
	
	return 0;
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

short loadMedia()
{
	//Load font texture
	if(LTexture_loadFromFile(&gBitmapTexture, "lazyfont.png")) {
		printf("Failed to load corner texture!\n");
		return -1;
	}

	//Build font from texture
	LBitmapFont_buildFont(&gBitmapFont, &gBitmapTexture);

	return 0;
}

void close_all()
{
	//Free loaded images
	LTexture_free(&gBitmapTexture);

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
		LBitmapFont_renderText(
				&gBitmapFont, 50, 10,
				"Bitmap Font:\n");
		LBitmapFont_renderText(
				&gBitmapFont, 50, 50,
				"ABDCEFGHIJKLMNOPQRSTUVWXYZ\n");
		LBitmapFont_renderText(
				&gBitmapFont, 100, 100,
				"abcdefghijklmnopqrstuvwxyz\n0123456789");

		//Update screen
		SDL_RenderPresent(gRenderer);
	}
equit:
	//Free resources and close SDL
	close_all();

	return 0;
}
