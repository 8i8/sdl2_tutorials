/*This source code copyrighted by Lazy Foo' Productions (2004-2015)
and may not be redistributed without written permission.*/

//Using SDL, SDL_image, standard IO, strings, and string streams
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <stdio.h>

//Screen dimension constants
const int SCREEN_WIDTH = 640;
const int SCREEN_HEIGHT = 480;

typedef struct {
	//The actual hardware texture
	SDL_Texture* mTexture;
	//Image dimensions
	int mWidth;
	int mHeight;
} LTexture;

typedef struct {
	//Window data
	SDL_Window* mWindow;
	//Window dimensions
	int mWidth;
	int mHeight;
	//Window focus
	int mMouseFocus;
	int mKeyboardFocus;
	int mFullScreen;
	int mMinimized;
} LWindow;

LWindow gWindow;		//Our custom window
SDL_Renderer* gRenderer = NULL;	//The window renderer
LTexture gSceneTexture;		//Scene textures

LTexture *free_texture(LTexture *lt)
{
	//Free texture if it exists
	if(lt->mTexture != NULL)
	{
		SDL_DestroyTexture(lt->mTexture);
		lt->mTexture = NULL;
		lt->mWidth = 0;
		lt->mHeight = 0;
	}
	return lt;
}

short LTexture_loadFromFile(LTexture *lt, char *path)
{
	//Get rid of preexisting texture
	free_texture(lt);

	//The final texture
	SDL_Texture* newTexture = NULL;

	//Load image at specified path
	SDL_Surface* loadedSurface = IMG_Load(path);
	if(loadedSurface == NULL) {
		printf("Unable to load image %s! SDL_image Error: %s\n",
				path, IMG_GetError());
		return -1;
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
		return -1;
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
	free_texture(lt);

	//Render text surface
	SDL_Surface* textSurface = TTF_RenderText_Solid(
						gFont, textureText, textColor);
	if(textSurface == NULL) {
		printf("Unable to render text surface! SDL_ttf Error: %s\n",
				TTF_GetError());
		return -1;
	}

	//Create texture from surface pixels
	lt->mTexture = SDL_CreateTextureFromSurface(gRenderer, textSurface);
	if(lt->mTexture == NULL) {
		printf("Unable to create texture from rendered text! SDL Error: %s\n",
				SDL_GetError());
		return -1;
	}

	//Get image dimensions
	lt->mWidth = textSurface->w;
	lt->mHeight = textSurface->h;

	//Get rid of old surface
	SDL_FreeSurface(textSurface);
	
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

short LTexture_render(LTexture *lt, int x, int y, SDL_Rect* clip)
{
	//Set rendering space and render to screen
	SDL_Rect renderQuad = {x, y, lt->mWidth, lt->mHeight};

	//Set clip rendering dimensions
	if(clip != NULL) {
		renderQuad.w = clip->w;
		renderQuad.h = clip->h;
	}

	//Render to screen
	return SDL_RenderCopy(gRenderer, lt->mTexture, clip, &renderQuad);
}

void LWindow_init(LWindow *w)
{
	w->mWindow = NULL;
	w->mMouseFocus = 1;
	w->mKeyboardFocus = 1;
	w->mFullScreen = 0;
	w->mMinimized = 0;
	w->mWidth = SCREEN_WIDTH;
	w->mHeight = SCREEN_HEIGHT;
}

SDL_Renderer* LWindow_createRenderer(LWindow *w)
{
	return SDL_CreateRenderer(
				w->mWindow,
				-1,
				SDL_RENDERER_ACCELERATED
				| SDL_RENDERER_PRESENTVSYNC);
}

void LWindow_handleEvent(LWindow *w, SDL_Event *e)
{
	if(e->type == SDL_WINDOWEVENT)
	{
		short updateCaption = 0;
		char caption[255];

		switch(e->window.event)
		{
			//Get new dimensions and repaint on window size change
			case SDL_WINDOWEVENT_SIZE_CHANGED:
			w->mWidth = e->window.data1;
			w->mHeight = e->window.data2;
			SDL_RenderPresent(gRenderer);
			break;

			//Repaint on exposure
			case SDL_WINDOWEVENT_EXPOSED:
			SDL_RenderPresent(gRenderer);
			break;

			//Mouse entered window
			case SDL_WINDOWEVENT_ENTER:
			w->mMouseFocus = 1;
			updateCaption = 1;
			break;
			
			//Mouse left window
			case SDL_WINDOWEVENT_LEAVE:
			w->mMouseFocus = 0;
			updateCaption = 1;
			break;

			//Window has keyboard focus
			case SDL_WINDOWEVENT_FOCUS_GAINED:
			w->mKeyboardFocus = 1;
			updateCaption = 1;
			break;

			//Window lost keyboard focus
			case SDL_WINDOWEVENT_FOCUS_LOST:
			w->mKeyboardFocus = 0;
			updateCaption = 1;
			break;

			//Window minimized
			case SDL_WINDOWEVENT_MINIMIZED:
			w->mMinimized = 1;
			break;

			//Window maxized
			case SDL_WINDOWEVENT_MAXIMIZED:
			w->mMinimized = 0;
			break;
			
			//Window restored
			case SDL_WINDOWEVENT_RESTORED:
			w->mMinimized = 0;
			break;
		}

		//Update window caption with new data
		if(updateCaption) {
			sprintf(caption, "%s %s %s %s",
					"SDL Tutorial - MouseFocus:",
					((w->mMouseFocus) ? "On" : "Off"),
					" KeyboardFocus:",
					((w->mKeyboardFocus) ? "On" : "Off"));
			SDL_SetWindowTitle(w->mWindow, caption);
		}
	}
	//Enter exit full screen on return key
	else if(e->type == SDL_KEYDOWN && e->key.keysym.sym == SDLK_RETURN)
	{
		if(w->mFullScreen) {
			SDL_SetWindowFullscreen(w->mWindow, SDL_FALSE);
			w->mFullScreen = 0;
		}
		else {
			SDL_SetWindowFullscreen(w->mWindow, SDL_TRUE);
			w->mFullScreen = 1;
			w->mMinimized = 0;
		}
	}
}

void LWindow_free(LWindow *w)
{
	if(w->mWindow != NULL)
		SDL_DestroyWindow(w->mWindow);

	w->mMouseFocus = 0;
	w->mKeyboardFocus = 0;
	w->mWidth = 0;
	w->mHeight = 0;
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
	LWindow_init(&gWindow);
	gWindow.mWindow = SDL_CreateWindow(
				"SDL Tutorial",
				SDL_WINDOWPOS_UNDEFINED,
				SDL_WINDOWPOS_UNDEFINED,
				SCREEN_WIDTH,
				SCREEN_HEIGHT,
				SDL_WINDOW_SHOWN
				| SDL_WINDOW_RESIZABLE);
	if(gWindow.mWindow == NULL) {
		printf("Window could not be created! SDL Error: %s\n",
				SDL_GetError());
		return -1;
	}

	//Create renderer for window
	gRenderer = LWindow_createRenderer(&gWindow);
	if(gRenderer == NULL) {
		printf("Renderer could not be created! SDL Error: %s\n",
				SDL_GetError());
		return -1;
	}

	//Initialize renderer color
	SDL_SetRenderDrawColor(gRenderer, 0xFF, 0xFF, 0xFF, 0xFF);

	//Initialize PNG loading
	if((IMG_Init(IMG_INIT_PNG) & IMG_INIT_PNG) == 0) {
		printf("SDL_image could not initialize! SDL_image Error: %s\n",
				IMG_GetError());
		return -1;
	}

	return 0;
}

short loadMedia()
{
	//Load scene texture
	if(LTexture_loadFromFile(&gSceneTexture, "window.png") < 0) {
		printf("Failed to load window texture!\n");
		return -1;
	}

	return 0;
}

void close_all()
{
	//Free loaded images
	free_texture(&gSceneTexture);

	//Destroy window	
	SDL_DestroyRenderer(gRenderer);
	LWindow_free(&gWindow);

	//Quit SDL subsystems
	IMG_Quit();
	SDL_Quit();
}

int main(int argc, char* argv[])
{
	//Start up SDL and create window
	if(init()) {
		printf("Failed to initialize!\n");
		goto equit;
	}

	//Load media
	if(loadMedia())	{
		printf("Failed to load media!\n");
		goto equit;
	}

	SDL_Event e;

	//While application is running
	while(1)
	{
		while(SDL_PollEvent(&e) != 0) {
			if(e.type == SDL_QUIT)
				goto equit;

			//Handle window events
			LWindow_handleEvent(&gWindow, &e);
		}

		//Only draw when not minimized
		if(!gWindow.mMinimized)
		{
			//Clear screen
			SDL_SetRenderDrawColor(gRenderer, 0xFF, 0xFF, 0xFF, 0xFF);
			SDL_RenderClear(gRenderer);

			//Render text textures
			LTexture_render(
					&gSceneTexture,
					(gWindow.mWidth - gSceneTexture.mWidth) / 2,
					(gWindow.mHeight - gSceneTexture.mHeight) / 2,
					NULL);

			//Update screen
			SDL_RenderPresent( gRenderer );
		}
	}
equit:
	//Free resources and close SDL
	close_all();

	return 0;
}
