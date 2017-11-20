/*This source code copyrighted by Lazy Foo' Productions (2004-2015)
and may not be redistributed without written permission.*/

//Using SDL, SDL_image, SDL_ttf, SDL_mixer, standard IO, math, and strings
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_mixer.h>
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
LTexture gPromptTexture;		//Scene texture
Mix_Music *gMusic = NULL;		//The music that will be played

//The sound effects that will be used
Mix_Chunk *gScratch = NULL;
Mix_Chunk *gHigh = NULL;
Mix_Chunk *gMedium = NULL;
Mix_Chunk *gLow = NULL;

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
		printf( "Unable to create texture from %s! SDL Error: %s\n",
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

short LTexture_render(LTexture *lt)
{
	return SDL_RenderCopy(gRenderer, lt->mTexture, NULL, NULL);
}

short init()
{
	//Initialize SDL
	if(SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO) < 0) {
		printf("SDL could not initialize! SDL Error: %s\n",
				SDL_GetError());
		return -1;
	}

	//Set texture filtering to linear
	if(SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "1") < 0)
		printf("Warning: Linear texture filtering not enabled!");

	//Create window
	gWindow = SDL_CreateWindow(
			"SDL Tutorial",
			SDL_WINDOWPOS_UNDEFINED,
			SDL_WINDOWPOS_UNDEFINED,
			SCREEN_WIDTH,SCREEN_HEIGHT,
			SDL_WINDOW_SHOWN);

	if(gWindow == NULL) {
		printf("Window could not be created! SDL Error: %s\n",
				SDL_GetError());
		return -1;
	}

	//Create vsynced renderer for window
	gRenderer = SDL_CreateRenderer(gWindow, -1, SDL_RENDERER_ACCELERATED);
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

	//Initialize SDL_mixer
	if(Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048) < 0) {
		printf("SDL_mixer could not initialize! SDL_mixer Error: %s\n",
				Mix_GetError());
		return -1;
	}

	return 0;
}

short loadMedia()
{
	//Load prompt texture
	if(LTexture_loadFromFile(&gPromptTexture, "prompt.png")) {
		printf( "Failed to load prompt texture!\n" );
		return -1;
	}

	//Load music
	gMusic = Mix_LoadMUS("beat.wav");
	if(gMusic == NULL) {
		printf( "Failed to load beat music! SDL_mixer Error: %s\n",
				Mix_GetError());
		return -1;
	}
	
	//Load sound effects
	gScratch = Mix_LoadWAV("scratch.wav");
	if(gScratch == NULL) {
		printf("Failed to load scratch sound effect! SDL_mixer Error: %s\n",
				Mix_GetError());
		return -1;
	}
	
	gHigh = Mix_LoadWAV("high.wav");
	if(gHigh == NULL) {
		printf("Failed to load high sound effect! SDL_mixer Error: %s\n",
				Mix_GetError());
		return -1;
	}

	gMedium = Mix_LoadWAV("medium.wav");
	if(gMedium == NULL) {
		printf("Failed to load medium sound effect! SDL_mixer Error: %s\n",
				Mix_GetError());
		return -1;
	}

	gLow = Mix_LoadWAV("low.wav");
	if(gLow == NULL) {
		printf( "Failed to load low sound effect! SDL_mixer Error: %s\n",
				Mix_GetError());
		return -1;
	}

	return 0;
}

void close_all()
{
	//Free loaded images
	free_texture(&gPromptTexture);

	//Free the sound effects
	Mix_FreeChunk(gScratch);
	Mix_FreeChunk(gHigh);
	Mix_FreeChunk(gMedium);
	Mix_FreeChunk(gLow);
	gScratch = NULL;
	gHigh = NULL;
	gMedium = NULL;
	gLow = NULL;
	
	//Free the music
	Mix_FreeMusic(gMusic);
	gMusic = NULL;

	//Destroy window	
	SDL_DestroyRenderer(gRenderer);
	SDL_DestroyWindow(gWindow);
	gWindow = NULL;
	gRenderer = NULL;

	//Quit SDL subsystems
	Mix_Quit();
	IMG_Quit();
	SDL_Quit();
}

void get_key_pressed(SDL_Event *e)
{
	switch(e->key.keysym.sym)
	{
		case SDLK_1:
			Mix_PlayChannel(-1, gHigh, 0);
			break;
		case SDLK_2:
			Mix_PlayChannel(-1, gMedium, 0);
			break;
		case SDLK_3:
			Mix_PlayChannel(-1, gLow, 0);
			break;
		case SDLK_4:
			Mix_PlayChannel(-1, gScratch, 0);
			break;
		case SDLK_9:
			if (Mix_PlayingMusic() == 0)
				Mix_PlayMusic(gMusic, -1);
			else if (Mix_PausedMusic() == 1)
				Mix_ResumeMusic();
			else
				Mix_PauseMusic();
			break;
		case SDLK_0:
			Mix_HaltMusic();
			break;
		default:
			break;
	}
}

int main(int argc, char* argv[])
{
	SDL_Event e;		//Event handler

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

	//While application is running
	while(1)
	{
		while(SDL_PollEvent(&e) != 0)
		{
			if(e.type == SDL_QUIT)
				goto equit;

			if(e.type == SDL_KEYDOWN)
				get_key_pressed(&e);
		}

		//Clear screen
		SDL_SetRenderDrawColor(gRenderer, 0xFF, 0xFF, 0xFF, 0xFF);
		SDL_RenderClear(gRenderer);

		//Render prompt
		LTexture_render(&gPromptTexture);

		//Update screen
		SDL_RenderPresent(gRenderer);
		SDL_Delay(16);
	}
equit:
	//Free resources and close SDL
	close_all();

	return 0;
}

