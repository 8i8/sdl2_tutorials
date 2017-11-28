/*
 * This program reads in a bitmap sprite sheet and uses those glyphs as a font.
 *
 * TODO find a way to simplifty/shorten the function that reads in all of the cha
 */
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <stdio.h>

#define SCREEN_WIDTH	640
#define SCREEN_HEIGHT	480

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

SDL_Window* gWindow = NULL;
SDL_Renderer* gRenderer = NULL;
LTexture gBitmapTexture;
LBitmapFont gBitmapFont;

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
		lt->mPixels = NULL;
		lt->mPitch = 0;
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
		SDL_Log("%s(), SDL_CreateTextureFromSurface failed.", __func__);
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
		SDL_Log("Texture already locked.");
		return -1;
	}

	if(SDL_LockTexture(lt->mTexture, NULL, &lt->mPixels, &lt->mPitch) != 0) {
		SDL_Log("%s(), SDL_LockTexture failed. %s",
				__func__, SDL_GetError());
		return -1;
	}
	return 0;
}

short LTexture_unlockTexture(LTexture *lt)
{
	if(lt->mPixels == NULL) {
		SDL_Log("Texture not locked.");
		return -1;
	}

	SDL_UnlockTexture(lt->mTexture);
	lt->mPixels = NULL;
	lt->mPitch = 0;

	return 0;
}

Uint32 LTexture_getPixel32(LTexture *lt, unsigned int x, unsigned int y)
{
	Uint32 *pixels = (Uint32*)lt->mPixels;

	return pixels[(y * (lt->mPitch / 4)) + x];
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
	if(LTexture_lockTexture(bitmap))
		return -1;

	Uint32 bgColor = LTexture_getPixel32(bitmap, 0, 0);

	int cellW = LTexture_getWidth(bitmap) / 16;
	int cellH = LTexture_getHeight(bitmap) / 16;

	int top = cellH;
	int baseA = cellH;

	int currentChar = 0;
	int rows, cols;
	int pCol, pRow;
	int pX, pY;

	for(rows = 0; rows < 16; ++rows) {
		for(cols = 0; cols < 16; ++cols)
		{
			lb->mChars[currentChar].x = cellW * cols;
			lb->mChars[currentChar].y = cellH * rows;

			lb->mChars[currentChar].w = cellW;
			lb->mChars[currentChar].h = cellH;

			for(pCol = 0; pCol < cellW; ++pCol)
			{
				for(pRow = 0; pRow < cellH; ++pRow)
				{
					pX = (cellW * cols) + pCol;
					pY = (cellH * rows) + pRow;

					if(LTexture_getPixel32(bitmap, pX, pY) != bgColor)
					{
						lb->mChars[currentChar].x = pX;

						pCol = cellW;
						pRow = cellH;
					}
				}
			}

			for(pCol = cellW - 1; pCol >= 0; --pCol)
			{
				for(pRow = 0; pRow < cellH; ++pRow)
				{
					pX = (cellW * cols) + pCol;
					pY = (cellH * rows) + pRow;

					if(LTexture_getPixel32(bitmap, pX, pY) != bgColor)
					{
						lb->mChars[currentChar].w = (pX - lb->mChars[currentChar].x) + 1;

						pCol = -1;
						pRow = cellH;
					}
				}
			}

			for(pRow = 0; pRow < cellH; ++pRow)
			{
				for(pCol = 0; pCol < cellW; ++pCol)
				{
					pX = (cellW * cols) + pCol;
					pY = (cellH * rows) + pRow;

					if(LTexture_getPixel32(bitmap, pX, pY) != bgColor)
					{
						if(pRow < top)
							top = pRow;

						pCol = cellW;
						pRow = cellH;
					}
				}
			}

			if(currentChar == 'A')
			{
				for(pRow = cellH - 1; pRow >= 0; --pRow)
				{
					for(pCol = 0; pCol < cellW; ++pCol)
					{
						pX = (cellW * cols) + pCol;
						pY = (cellH * rows) + pRow;

						if(LTexture_getPixel32(bitmap, pX, pY) != bgColor)
						{
							baseA = pRow;

							pCol = cellW;
							pRow = -1;
						}
					}
				}
			}

			++currentChar;
		}
	}

	lb->mSpace = cellW / 2;
	lb->mNewLine = baseA - top;

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
	if(lb->mBitmap == NULL)
		return -1;

	int curX = x, curY = y;

	for(unsigned i = 0; i < strlen(text); ++i)
	{
		if(text[i] == ' ')
			curX += lb->mSpace;
		else if(text[i] == '\n')
		{
			curY += lb->mNewLine;

			curX = x;
		}
		else
		{
			int ascii = (unsigned char)text[i];

			LTexture_render(
					lb->mBitmap,
					curX, curY,
					&lb->mChars[ascii],
					0.0, NULL, 0);

			curX += lb->mChars[ascii].w + 1;
		}
	}
	
	return 0;
}

short loadMedia()
{
	if(LTexture_loadFromFile(&gBitmapTexture, "lazyfont.png"))
		return -1;

	LBitmapFont_buildFont(&gBitmapFont, &gBitmapTexture);

	return 0;
}

void close_all()
{
	LTexture_free(&gBitmapTexture);

	SDL_DestroyRenderer(gRenderer);
	SDL_DestroyWindow(gWindow);
	gWindow = NULL;
	gRenderer = NULL;

	IMG_Quit();
	SDL_Quit();
}

int main(int argc, char* args[])
{
	if(init())
		goto equit;

	if(loadMedia())
		goto equit;

	SDL_Event e;

	while(1)
	{
		while(SDL_PollEvent(&e) != 0)
			if(e.type == SDL_QUIT)
				goto equit;

		SDL_SetRenderDrawColor(gRenderer, 0xFF, 0xFF, 0xFF, 0xFF);
		SDL_RenderClear(gRenderer);

		LBitmapFont_renderText(
				&gBitmapFont, 50, 10,
				"Bitmap Font:\n");
		LBitmapFont_renderText(
				&gBitmapFont, 50, 50,
				"ABDCEFGHIJKLMNOPQRSTUVWXYZ\n");
		LBitmapFont_renderText(
				&gBitmapFont, 100, 100,
				"abcdefghijklmnopqrstuvwxyz\n0123456789");

		SDL_RenderPresent(gRenderer);
	}
equit:
	close_all();

	return 0;
}
