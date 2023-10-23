/*
 * Color Modulation
 *
 * Color modulation allows you to alter the color of your rendered textures.
 * Here we're going to modulate a texture using various colors.
 *
 * We're adding a function to the texture wrapper that will allow the texture
 * modulation to be set. All it does is take in a red, green, and blue color
 * components.
 */
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>

#define SCREEN_WIDTH	640
#define SCREEN_HEIGHT	480

typedef struct {
	SDL_Texture *mTexture;
	int mWidth;
	int mHeight;
} LTexture;

typedef struct {
	Uint8 r;
	Uint8 g;
	Uint8 b;
} Colours;

SDL_Window* gWindow = NULL;
SDL_Renderer* gRenderer = NULL;
LTexture gModulatedTexture;

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

	gRenderer = SDL_CreateRenderer(gWindow, -1, SDL_RENDERER_ACCELERATED);
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

short loadFromFile(LTexture *lt, char *path)
{
	free_texture(lt);

	SDL_Texture* newTexture = NULL;

	SDL_Surface* loadedSurface = IMG_Load(path);
	if(loadedSurface == NULL) {
		SDL_Log("%s(), IMG_Load failed. %s", __func__, IMG_GetError());

		return -1;
	}

	SDL_SetColorKey(loadedSurface, SDL_TRUE, SDL_MapRGB(
				loadedSurface->format, 0, 0xFF, 0xFF));

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

/*
 * And setting texture modulation is as easy as making a call to
 * SDL_SetTextureColorMod. You just pass in the texture you want to modulate
 * and the color you want to modulate with.
 *
 * You may have noticed that SDL_SetTextureColorMod accepts Uint8 as arguments
 * for the color components. An Uint8 is just an integer that is Unsigned and
 * 8bit. This means it goes from 0 to 255. 128 is about halfway between 0 and
 * 255, so when you modulate green to 128 it halves the green component for any
 * pixel on the texture.
 *
 * The red and blue squares don't get affected because they have no green in
 * them, but the green becomes half as bright and the white turns a light
 * magenta (magenta is red 255, green 0, blue 255). Color modulation is just a
 * way to multiply a color throughout the whole texture.
 */
void LTexture_setColor(LTexture *lt, Uint8 red, Uint8 green, Uint8 blue)
{
	SDL_SetTextureColorMod(lt->mTexture, red, green, blue);
}

short LTexture_render(LTexture *lt, int x, int y, SDL_Rect* clip)
{
	SDL_Rect renderQuad = {x, y, lt->mWidth, lt->mHeight};

	if(clip != NULL) {
		renderQuad.w = clip->w;
		renderQuad.h = clip->h;
	}

	return SDL_RenderCopy(gRenderer, lt->mTexture, clip, &renderQuad);
}

short loadMedia(void)
{
	if(loadFromFile(&gModulatedTexture, "colors.png"))
		return -1;

	return 0;
}

void close_all(void)
{
	free_texture(&gModulatedTexture);

	SDL_DestroyRenderer(gRenderer);
	SDL_DestroyWindow(gWindow);
	gWindow = NULL;
	gRenderer = NULL;

	IMG_Quit();
	SDL_Quit();
}

/*
 * In our event loop we'll have the q, w, and e keys increase the red, green,
 * and blue components and we'll have the a, s, and d key decrease the red,
 * green, and blue components. They increase/decrease the components by 32 so
 * it's noticable with every key press.
 */
void get_event(SDL_Event *e, Colours *c)
{
	if(e->type == SDL_KEYDOWN)
	{
		switch(e->key.keysym.sym)
		{
			case SDLK_q:
			c->r += 32;
			break;
			
			case SDLK_w:
			c->g += 32;
			break;
			
			case SDLK_e:
			c->b += 32;
			break;
			
			case SDLK_a:
			c->r -= 32;
			break;
			
			case SDLK_s:
			c->g -= 32;
			break;
			
			case SDLK_d:
			c->b -= 32;
			break;
		}
	}
}

/*
 * Here we are right before the main loop. For this demo we're going to
 * modulate the individual color components using key presses. To do that we'll
 * need to keep track of the values for the color components, to do this we
 * create a struct in which to store the colour values.
 */
int main(int argc, char* argv[])
{
	SDL_Event e;
	SDL_Rect* clip = NULL;
	Colours c = { 255, 255, 255 };

	if(init())
		goto equit;

	if(loadMedia())
		goto equit;

	while(1)
	{
		while(SDL_PollEvent(&e) != 0) {
			if(e.type == SDL_QUIT)
				goto equit;

			get_event(&e, &c);
		}

		SDL_SetRenderDrawColor(gRenderer, 0xFF, 0xFF, 0xFF, 0xFF);
		SDL_RenderClear(gRenderer);
/*
 * Here we are setting the texture modulation and rendering the texture. 
 */
		LTexture_setColor(&gModulatedTexture, c.r, c.g, c.b);
		LTexture_render(&gModulatedTexture, 0, 0, clip);

		SDL_RenderPresent(gRenderer);

		SDL_Delay(16);
	}
equit:
	close_all();

	return 0;
}

