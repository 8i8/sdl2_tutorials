/*
 * Bitmap Fonts
 *
 * Some times TTF fonts are flexible enough. Since rendering text is just
 * rendering images of characters, we can use bitmap fonts to render text.
 *
 * If you think of each character in a string as a sprite, you can think of
 * font rendering as arranging a bunch of sprites; Bitmap fonts work by taking
 * a sprite sheet of glyphs (character images) and rendering them in order to
 * form strings on the screen. 
 *
 * In previous tutorials when we did texture pixel manipulation, we didn't care
 * which pixel we got since we wanted to grab all the pixels. Here we need to
 * get pixels at exact x/y coordinates which is why we're adding a getPixel32
 * function. This function works specifically for 32bit pixels.
 */
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>

#define SCREEN_WIDTH	640
#define SCREEN_HEIGHT	480

typedef struct {
	SDL_Texture* mTexture;
	void* mPixels;
	int mPitch;
	int mWidth;
	int mHeight;
} LTexture;

/*
 * Here is our bitmap font which functions as a wrapper for a sprite sheet of
 * glyphs. It has a constructor to initialize internal variables, a function to
 * build the font, and a function to render the text.
 *
 * When the bitmap font is built we go through the texture a find all the
 * character sprites for the 256 characters (that are stored in the mChars
 * array) and calculate the distance for a new line and a space.
 */
typedef struct {
	LTexture* mBitmap;
	SDL_Rect mChars[256];
	int mNewLine, mSpace;
} LBitmapFont;

SDL_Window* gWindow = NULL;
SDL_Renderer* gRenderer = NULL;
LTexture gBitmapTexture;
LBitmapFont gBitmapFont;

short init(void)
{
	if(SDL_Init(SDL_INIT_VIDEO) < 0) {
		SDL_Log("%s(), SDL_Init failed. %s", __func__, SDL_GetError());
		return -1;
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
		lt->mPixels = NULL;
		lt->mPitch = 0;
	}
}

/*
 * Here is our texture loading from the previous tutorial with some more
 * tweaks. We did the color keying externally in the previous tutorial, and
 * here we're doing it internally in the texture loading function.
 *
 * Secondly, we're specifying the texture pixel format as
 * SDL_PIXELFORMAT_RGBA8888 so we know we'll get 32bit RGBA pixels.
 */
short LTexture_loadFromFile(LTexture *lt, char *path)
{
	LTexture_free(lt);

	SDL_Texture* newTexture = NULL;

	SDL_Surface* loadedSurface = IMG_Load(path);
	if(loadedSurface == NULL) {
		SDL_Log("%s(), IMG_Load failed. %s", __func__, IMG_GetError());

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
		SDL_Log("%s(), SDL_CreateTextureFromSurface failed. %s", __func__, SDL_GetError());
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

/*
 * Here is our function to get a pixel at a specific offset.
 *
 * The important thing to know is that even though we have a 2 dimensional
 * texture image like a grid or a matrix; Pixels are stored in a one
 * dimensionial array.
 *
 * So if you wanted to get the blue pixel in row 1, column 1 (the first
 * row/column is row/column 0), you would have to calculate the offset like
 * this:
 * Y Offset * Pitch + X Offset
 *
 * Which comes out to:
 * 1 * 5 + 1 = 6
 *
 * And as you can see, the pixel at index 6 on the 1 dimensional pixels is the
 * same as the one on row 1 column 1 on the 2 dimensional pixels.
 *
 * And if you're wondering why we divide the pitch by 4, remember that the
 * pitch is in bytes. Since we need the pitch in pixels and there's 4 bytes per
 * pixel, we divide the pitch by 4.
 */
Uint32 LTexture_getPixel32(LTexture *lt, unsigned int x, unsigned int y)
{
	Uint32 *pixels = (Uint32*)lt->mPixels;

	return pixels[(y * (lt->mPitch / 4)) + x];
}

/*
 * Now we're entering the function that's going to go through the bitmap font
 * and define all the clip rectanges for all the sprites. To do that we'll have
 * to lock the texture to access its pixels.
 *
 * In order for this bitmap font loading to work, the character glyphs need to
 * be arranged in cells; The cells all need to all have the same width and
 * height, arranged in 16 columns and 16 rows, and need to be in ASCII order.
 * The bitmap font loader is going to go through each of the cells, find the
 * sides of the glyph sprites and set the clip rectangle for the sprite.
 * 
 * First we get the background color which we'll need to find the edges of the
 * glyph sprites. Then we calculate the cell width and height. We have the
 * variable called top which will keep track of the top of the tallest glyph in
 * the sprite sheet. The variable baseA will keep track of the offset of the
 * bottom of the capital A glyph which will use as a base line for rendering
 * characters.
 * 
 * Lastly we have the currentChar which keeps track of the current character
 * glyph we're looking for.
 */
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
/*
 * These two nested for loops are for going through the cell rows/columns.
 *
 * At the top of per cell loop, we initialize the glyph sprite positon at the
 * top of the cell and the sprite dimensions to be the cell dimensions. This
 * means by default the glyph sprite is the full cell.
 */
	for(rows = 0; rows < 16; ++rows) {
		for(cols = 0; cols < 16; ++cols)
		{
			lb->mChars[currentChar].x = cellW * cols;
			lb->mChars[currentChar].y = cellH * rows;

			lb->mChars[currentChar].w = cellW;
			lb->mChars[currentChar].h = cellH;
/*
 * For each cell we need to go through all the pixels in the cell to find the
 * edge of the glyph sprite. In this loop we go through each column from top to
 * bottom and look for the first pixel that is not the background color. Once
 * we find a pixel that is not the background color it means we found the left
 * edge of the sprite:
 *
 * When we find the left side of the glyph we set it as x position of the
 * sprite and then break the loops. 
 */
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
/*
 * Here we're looking for the pixel on the right side. It works pretty much the
 * same as finding the left side, only now we're moving from right to left
 * instead of left to right.
 *
 * When we find the right pixel, we use it to set the width. Since the pixel
 * array starts at 0, we need to add 1 to the width.
 */
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
/*
 * Here is the code to find the top of the sprite. When we find a top that is
 * higher that the current highest top, we set it as the new top.
 *
 * Note that since the y axis is inverted, the highest top actually has the
 * lowest y offset. 
 */
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
/*
 * In terms of looking for the bottom of the glyphs, the only one we care about
 * is the capital A. For this bitmap font builder we're going to use the bottom
 * of the A glyph sprite as the base line so characters like "g", "j", "y", etc
 * that hang below the baseline don't define the bottom. You don't have to do
 * it this way, but it's given me good results before.
 */
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
/*
 * After we're done defining all the sprites, we have some post processing to
 * do. First we calculate how long a space is. Here we're defining it as half a
 * cell width. We then calculate the height of a new line by using the baseline
 * and the highest sprite top.
 *
 * We then lop off the extra space at the top of each glyph to prevent there
 * from being too much space between lines. Finally we unlock the texture and
 * set the bitmap for the bitmap font.
 *
 * Now the way we constructed the bitmap font isn't the only way to do it. You
 * can define spaces, new lines, and base lines another way. You use an XML
 * file to define the positions of the sprites instead of using cells. I
 * decided to go with this method because it's a common one and it has worked
 * for me.
 */
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

/*
 * Now that we have all the glyph sprites defined, it's time to render them to
 * the screen. First we check that there is a bitmap to render with, then we
 * declare x/y offsets that we'll be using to render the current glyph sprite. 
 */
short LBitmapFont_renderText(LBitmapFont *lb, int x, int y, char *text)
{
	if(lb->mBitmap == NULL)
		return -1;

	int curX = x, curY = y;

/*
 * Here is the for loop that goes through the string to render each glyph
 * sprite. However there are two ASCII values we're not actually going to
 * render anything for. When we have a space, all we have to do is move over
 * the space width. When we have a new line we move down a new line and back to
 * the base x offset.
 */
	size_t i;
	for(i = 0; i < strlen(text); ++i)
	{
		if(text[i] == ' ')
			curX += lb->mSpace;
		else if(text[i] == '\n')
		{
			curY += lb->mNewLine;

			curX = x;
		}
		else
/*
 * For nonspecial characters, we render the sprite. As you can see, it's as
 * simple as getting the ASCII value, rendering the sprite associated with the
 * ASCII value and then moving over the width of the sprite.
 *
 * The for loop will then keep going through all the characters and rendering
 * the sprite for each of them one after the other. 
 */
		{
			int ascii = text[i];

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

short loadMedia(void)
{
	if(LTexture_loadFromFile(&gBitmapTexture, "lazyfont.png"))
		return -1;

	LBitmapFont_buildFont(&gBitmapFont, &gBitmapTexture);

	return 0;
}

void close_all(void)
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

