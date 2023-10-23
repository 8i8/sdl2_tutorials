/*
 * True Type Fonts
 *
 * One way to render text with SDL is with the extension library SDL_ttf.
 * SDL_ttf allows you to create images from TrueType fonts which we'll use here
 * to create textures from font text.
 *
 * To use SDL_ttf, you have to set up the SDL_ttf extension library just like
 * you would set up SDL_image. Like before, it's just a matter of having the
 * headers files, library files, and binary files in the right place with your
 * compiler configured to use them.
 *
 * Here we're adding another function for the texture called
 * LTexture_loadFromRenderedText. The way SDL_ttf works is that you create a
 * new image from a font and color. For our texture class all that means is
 * that we're going to be loading our image from text rendered by SDL_ttf
 * instead of a file.
 */
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_ttf.h>

#define SCREEN_WIDTH	640
#define SCREEN_HEIGHT	480

typedef struct {
	SDL_Texture *mTexture;
	int mWidth;
	int mHeight;
} LTexture;

SDL_Window* gWindow = NULL;
SDL_Renderer* gRenderer = NULL;

/*
 * For this and future tutorials, we'll be using a global font for our text
 * rendering. In SDL_ttf, the data type for fonts is TTF_Font. We also have a
 * texture which will be generated from the font.
 */
TTF_Font *gFont = NULL;
LTexture gTextTexture;

short init(void)
{
	if(SDL_Init(SDL_INIT_VIDEO) < 0) {
		SDL_Log("%s(), SDL_Init failed. %s", __func__, SDL_GetError());
		return -1;
	}

	/* TODO test the effect of this filter on text. */
	if(SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "1") == 0)
		SDL_Log("Warning: Linear texture filtering failed.");

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
/*
 * Just like SDL_image, we have to initialize it or the font loading and
 * rendering functions won't work properly. We start up SDL_ttf using TTF_init.
 * We can check for errors using TTF_GetError().
 */
 	if(TTF_Init() < 0) {
		SDL_Log("%s(), TTF_Init failed. %s", __func__, TTF_GetError());
		return -1;
	}

	return 0;
}

LTexture *free_texture(LTexture *lt)
{
	if(lt->mTexture != NULL)
	{
		SDL_DestroyTexture(lt->mTexture);
		lt->mTexture = NULL;
		lt->mWidth = 0;
		lt->mHeight = 0;
	}
	return lt;
}

/*
 * Here is where we actually create the text texture we're going to render from
 * the font. This function takes in the string of text we want to render and
 * the color we want to use to render it. After that, this function pretty much
 * works like loading from a file does, only this time we're using a
 * SDL_Surface created by SDL_ttf instead of a file.
 *
 * After freeing any preexisting textures, we load a surface using
 * TTF_RenderText_Solid. This creates a solid color surface from the font,
 * text, and color given. If the surface was created successfully, we create a
 * texture out of it just like we did before when loading a surface from a
 * file. After the text texture is created, we can render with it just like any
 * other texture.
 *
 * There are other ways to render text that are smoother or blended. Experiment
 * with the different types of rendering outlined in the SDL_ttf documentation. 
 *
 * https://www.libsdl.org/projects/docs/SDL_ttf/SDL_ttf_35.html
 */
short LTexture_loadFromRenderedText(
					LTexture *lt,
					char *textureText, 
					SDL_Color textColor)
{
	free_texture(lt);

	SDL_Surface* textSurface = TTF_RenderText_Solid(
			gFont, textureText, textColor);
	if(textSurface == NULL) {
		SDL_Log("%s(), TTF_RenderText_Solid failed. %s", __func__, TTF_GetError());
		return -1;
	}

	lt->mTexture = SDL_CreateTextureFromSurface(gRenderer, textSurface);
	if(lt->mTexture == NULL) {
		SDL_Log("%s(), SDL_CreateTextureFromSurface failed. %s", __func__, SDL_GetError());

		return -1;
	}

	lt->mWidth = textSurface->w;
	lt->mHeight = textSurface->h;

	SDL_FreeSurface(textSurface);
	
	return 0;
}

short LTexture_render(
			LTexture *lt,
			int x,
			int y,
			SDL_Rect *clip)
{
	SDL_Rect renderQuad = {x, y, lt->mWidth, lt->mHeight};

	if(clip != NULL) {
		renderQuad.w = clip->w;
		renderQuad.h = clip->h;
	}

	return SDL_RenderCopy(gRenderer, lt->mTexture, clip, &renderQuad);
}

int LTexture_getWidth(LTexture *lt)
{
	return lt->mWidth;
}

int LTexture_getHeight(LTexture *lt)
{
	return lt->mHeight;
}

/*
 * In our loading function, we load our font using TTF_OpenFont. This takes in
 * the path to the font file and the point size we want to render at.
 *
 * If the font loaded successfully, we want to load a text texture using our
 * loading method. As a general rule, you want to minimize the number of time
 * you render text. Only rerender it when you need to and since we're using the
 * same text surface for this whole program, we only want to render once.
 */
short loadMedia(void)
{
	gFont = TTF_OpenFont("DejaVuSerif.ttf", 28);
	if(gFont == NULL) {
		SDL_Log("%s(), TTF_OpenFont failed. %s", __func__, TTF_GetError());
		return -1;
	}

	SDL_Color textColor = {0, 0, 0, 0};
	if(LTexture_loadFromRenderedText(
				&gTextTexture,
				"The quick brown fox jumps over the lazy dog",
				textColor))
		return -1;

	return 0;
}

/*
 * In our clean up function, we want to free the font using TTF_CloseFont. We
 * also want to quit the SDL_ttf library with TTF_Quit to complete the clean
 * up.
 */
void close_all(void)
{
	free_texture(&gTextTexture);

	TTF_CloseFont(gFont);
	gFont = NULL;

	SDL_DestroyRenderer(gRenderer);
	SDL_DestroyWindow(gWindow);
	gWindow = NULL;
	gRenderer = NULL;

	TTF_Quit();
	IMG_Quit();
	SDL_Quit();
}

int main(int argc, char* args[])
{
	SDL_Event e;

	if(init())
		goto equit;

	if(loadMedia())
		goto equit;

	while(1)
	{
		while(SDL_PollEvent(&e) != 0)
			if(e.type == SDL_QUIT)
				goto equit;

		SDL_SetRenderDrawColor(gRenderer, 0xFF, 0xFF, 0xFF, 0xFF);
		SDL_RenderClear(gRenderer);
/*
 * As you can see, after we render the text texture we can render it just like
 * any other texture.
 */
		LTexture_render(
				&gTextTexture,
				(SCREEN_WIDTH - LTexture_getWidth(
						&gTextTexture)) / 2,
				(SCREEN_HEIGHT - LTexture_getHeight(
						&gTextTexture)) / 2,
				NULL);

		SDL_RenderPresent(gRenderer);
	}
equit:
	close_all();

	return 0;
}

