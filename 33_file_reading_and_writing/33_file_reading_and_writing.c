/*This source code copyrighted by Lazy Foo' Productions (2004-2015)
and may not be redistributed without written permission.*/

//Using SDL, SDL_image, SDL_ttf, standard IO, strings, and string streams
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_ttf.h>
#include <SDL2/SDL_rwops.h>
#include <SDL2/SDL_filesystem.h>
#include <stdio.h>

typedef long long _Longlong;

//Screen dimension constants
const int SCREEN_WIDTH = 640;
const int SCREEN_HEIGHT = 480;

//Number of data integers
#define TOTAL_DATA	10

typedef struct {
	SDL_Texture *mTexture;
	int mWidth;
	int mHeight;
} LTexture;

typedef struct {
	SDL_Color textColor;
	SDL_Color highlightColor;
	int currentData;
} STdata;

SDL_Window* gWindow = NULL;		//The window we'll be rendering to
SDL_Renderer* gRenderer = NULL;		//The window renderer
TTF_Font *gFont = NULL;			//Globally used font
LTexture gPromptTextTexture;		//Scene textures
LTexture gDataTextures[TOTAL_DATA];
STdata gStr;
Sint32 gData[TOTAL_DATA];		//Data points

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

void gStr_init(STdata *gStr)
{
	SDL_Color textColor = { 0, 0, 0, 0xFF };
	SDL_Color highlightColor = { 0xFF, 0, 0, 0xFF };

	gStr->textColor = textColor;
	gStr->highlightColor = highlightColor;
	gStr->currentData = 0;
}

short loadMedia()
{
	int i;
	char string[255];

	//Text rendering color
	gStr_init(&gStr);

	//Open the font
	gFont = TTF_OpenFont("lazy.ttf", 28);
	if(gFont == NULL) {
		printf( "Failed to load lazy font! SDL_ttf Error: %s\n",
				TTF_GetError());
		return -1;
	}

	//Render the prompt
	if(LTexture_loadFromRenderedText(
				&gPromptTextTexture,
				"Enter Data:",
				gStr.textColor) < 0)
	{
		printf("Failed to render prompt text!\n");
		return -1;
	}

	//Open file for reading in binary
	SDL_RWops* file = SDL_RWFromFile("nums.bin", "r+b");

	//File does not exist
	if(file == NULL)
	{
		if((file = SDL_RWFromFile("nums.bin", "w+b")) == NULL) {
			printf("Error: Unable to create file! SDL Error: %s\n",
					SDL_GetError());
			return -1;
		}
		printf("New file created!\n");

		//Initialize data
		for(i = 0; i < TOTAL_DATA; ++i) {
			gData[i] = 0;	
			SDL_RWwrite(file, &gData[i], sizeof(Sint32), 1);
		}
		
		//Close file handler
		SDL_RWclose(file);
	}
	else
	{
		//Load data
		printf( "Reading file...!\n" );
		for(i = 0; i < TOTAL_DATA; ++i )
			SDL_RWread( file, &gData[ i ], sizeof(Sint32), 1 );

		//Close file handler
		SDL_RWclose( file );
	}

	//Initialize data textures
	sprintf(string, "%d", gData[0]);
	LTexture_loadFromRenderedText(
				&gDataTextures[0],
				string,
				gStr.highlightColor);

	for(i = 1; i < TOTAL_DATA; ++i) {
		sprintf(string, "%d", gData[i]);
		LTexture_loadFromRenderedText(
					&gDataTextures[i],
					string,
					gStr.textColor);
	}

	return 0;
}

void close_all()
{
	int i;
	SDL_RWops* file = SDL_RWFromFile( "nums.bin", "w+b" );
	if(file != NULL) {
		for(i = 0; i < TOTAL_DATA; ++i )
			SDL_RWwrite( file, &gData[ i ], sizeof(Sint32), 1 );

		SDL_RWclose( file );
	}
	else
		printf( "Error: Unable to save file! %s\n", SDL_GetError() );

	//Free loaded images
	free_texture(&gPromptTextTexture);
	for(i = 0; i < TOTAL_DATA; ++i)
		free_texture(&gDataTextures[i]);

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

void get_key(SDL_Event *e, STdata *str)
{
	char string[255];
	switch(e->key.keysym.sym)
	{
		//Previous data entry
		case SDLK_UP:
			//Rerender previous entry input point
			sprintf(string, "%d", gData[str->currentData]);
			LTexture_loadFromRenderedText(
					&gDataTextures[str->currentData],
					string,
					str->textColor);
			--(str->currentData);
			if(str->currentData < 0 )
				str->currentData = TOTAL_DATA - 1;
			
			//Rerender current entry input point
			sprintf(string, "%d", gData[str->currentData]);
			LTexture_loadFromRenderedText(
					&gDataTextures[str->currentData],
					string,
					str->highlightColor);
			break;
		
		//Next data entry
		case SDLK_DOWN:
			//Rerender previous entry input point
			sprintf(string, "%d", gData[str->currentData]);
			LTexture_loadFromRenderedText(
					&gDataTextures[str->currentData],
					string,		
					str->textColor);
			++(str->currentData);
			if(str->currentData == TOTAL_DATA)
				str->currentData = 0;
			
			//Rerender current entry input point
			sprintf(string, "%d", gData[str->currentData]);
			LTexture_loadFromRenderedText(
					&gDataTextures[str->currentData],
					string,
					str->highlightColor);
			break;

		//Decrement input point
		case SDLK_LEFT:
			--gData[str->currentData];
			sprintf(string, "%d", gData[str->currentData]);
			LTexture_loadFromRenderedText(
					&gDataTextures[str->currentData],
					string,
					str->highlightColor);
			break;
		
		//Increment input point
		case SDLK_RIGHT:
			++gData[str->currentData];
			sprintf(string, "%d", gData[str->currentData]);
			LTexture_loadFromRenderedText(
					&gDataTextures[str->currentData],
					string,
					str->highlightColor);
			break;
	}
}

int main(int argc, char* args[])
{
	int i;

	//Start up SDL and create window
	if(init()) {
		printf( "Failed to initialize!\n" );
		goto equit;
	}

	//Load media
	if(loadMedia()) {
		printf( "Failed to load media!\n" );
		goto equit;
	}

	SDL_Event e;

	while(1)
	{
		while(SDL_PollEvent(&e) != 0) {
			if(e.type == SDL_QUIT)
				goto equit;

			if(e.type == SDL_KEYDOWN)
				get_key(&e, &gStr);
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
		for(i = 0; i < TOTAL_DATA; ++i)
			LTexture_render(
					&gDataTextures[i],
					(SCREEN_WIDTH - gDataTextures[i].mWidth) / 2,
					gPromptTextTexture.mHeight
					+ gDataTextures[0].mHeight * i,
					NULL);

		//Update screen
		SDL_RenderPresent( gRenderer );
	}
equit:
	//Free resources and close SDL
	close_all();

	return 0;
}
