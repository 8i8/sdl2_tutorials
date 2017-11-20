/*This source code copyrighted by Lazy Foo' Productions (2004-2015)
and may not be redistributed without written permission.*/

//Using SDL, SDL_image, SDL_ttf, standard IO, strings, and string streams
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_ttf.h>
#include <stdio.h>

//Screen dimension constants
const int SCREEN_WIDTH = 640;
const int SCREEN_HEIGHT = 480;

typedef struct {
	SDL_Texture *mTexture;
	int mWidth;
	int mHeight;
} LTexture;

SDL_Window* gWindow = NULL;		//The window we'll be rendering to
SDL_Renderer* gRenderer = NULL;		//The window renderer
TTF_Font *gFont = NULL;			//Globally used font
LTexture gPromptTextTexture;		//Scene textures
LTexture gInputTextTexture;

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

short init()
{
	//Initialize SDL
	if(SDL_Init( SDL_INIT_VIDEO ) < 0) {
		printf("SDL could not initialize! SDL Error: %s\n",
				SDL_GetError());
		return -1;
	}

	//Set texture filtering to linear
	if(SDL_SetHint( SDL_HINT_RENDER_SCALE_QUALITY, "1") == 0)
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

	//Create vsynced renderer for window
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
	if((IMG_Init(IMG_INIT_PNG) & IMG_INIT_PNG) == 0) {
		printf( "SDL_image could not initialize! SDL_image Error: %s\n",
				IMG_GetError());
		return -1;
	}

	 //Initialize SDL_ttf
	if(TTF_Init() == -1) {
		printf( "SDL_ttf could not initialize! SDL_ttf Error: %s\n",
				TTF_GetError());
		return -1;
	}

	return 0;
}

short loadMedia()
{
	//Open the font
	gFont = TTF_OpenFont("lazy.ttf", 28);
	if(gFont == NULL) {
		printf("Failed to load lazy font! SDL_ttf Error: %s\n",
				TTF_GetError() );
		return -1;
	}

	//Render the prompt
	SDL_Color textColor = { 0, 0, 0, 0xFF };
	if(LTexture_loadFromRenderedText(
						&gPromptTextTexture,
						"Enter Text:",
						textColor))
	{
		printf( "Failed to render prompt text!\n" );
		return -1;
	}

	return 0;
}

void close_all()
{
	//Free loaded images
	free_texture(&gPromptTextTexture);
	free_texture(&gInputTextTexture);

	//Free global font
	TTF_CloseFont(gFont);
	gFont = NULL;

	//Destroy window	
	SDL_DestroyRenderer(gRenderer);
	SDL_DestroyWindow(gWindow);
	gWindow = NULL;
	gRenderer = NULL;

	//Quit SDL subsystems
	TTF_Quit();
	IMG_Quit();
	SDL_Quit();
}

short get_input(SDL_Event *e, char *inputText)
{
	int len = 0;
	len = strlen(inputText);

	if(e->type == SDL_KEYDOWN)
	{
		//Handle backspace
		if(e->key.keysym.sym == SDLK_BACKSPACE && len > 0) {
			*(inputText+len-1) = '\0';
			return 1;
		} //Handle copy
		else if(e->key.keysym.sym ==
				SDLK_c && SDL_GetModState() & KMOD_CTRL) {
			SDL_SetClipboardText(inputText);
		} //Handle paste
		else if(e->key.keysym.sym ==
				SDLK_v && SDL_GetModState() & KMOD_CTRL) {
			strcpy(inputText, SDL_GetClipboardText());
			return 1;
		}
	}
	else if(e->type == SDL_TEXTINPUT) { //Not copy or pasting
		if(
					((e->text.text[0] == 'c'
					|| e->text.text[0] == 'C')
					&& (e->text.text[0] == 'v'
					|| e->text.text[0] == 'V')
					&& SDL_GetModState()
					& KMOD_CTRL)
					== 0)
		{
			//Append character
			strcat(inputText, e->text.text);
			return 1;
		}
	}
	return 0;
}

int main(int argc, char* argv[])
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
	SDL_Color textColor = { 0, 0, 0, 0xFF };

	//The current input text.
	char inputText[255] = "Some Text";
	LTexture_loadFromRenderedText(
			&gInputTextTexture, inputText, textColor);

	//Enable text input
	SDL_StartTextInput();

	//While application is running
	while(1)
	{
		//The rerender text flag
		short renderText = 0;

		while(SDL_PollEvent(&e) != 0) {
			if(e.type == SDL_QUIT)
				goto equit;

			renderText = get_input(&e, inputText);
		}

		//Rerender text if needed
		if(renderText) {
			if(strcmp(inputText, ""))
				LTexture_loadFromRenderedText(
							&gInputTextTexture,
							inputText,
							textColor);
			else
				LTexture_loadFromRenderedText(
							&gInputTextTexture,
							" ",
							textColor);
		}

		//Clear screen
		SDL_SetRenderDrawColor(gRenderer, 0xFF, 0xFF, 0xFF, 0xFF);
		SDL_RenderClear(gRenderer);

		//Render text textures
		LTexture_render(
				&gPromptTextTexture,
				(SCREEN_WIDTH - gPromptTextTexture.mWidth) / 2,
				0,
				NULL);
		LTexture_render(
				&gInputTextTexture,
				(SCREEN_WIDTH - gInputTextTexture.mWidth) / 2,
				gPromptTextTexture.mHeight,
				NULL);

		//Update screen
		SDL_RenderPresent(gRenderer);
	}
	
	//Disable text input
	SDL_StopTextInput();
equit:
	//Free resources and close SDL
	close_all();

	return 0;
}
