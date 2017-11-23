/*
 * This program demonstrates the playing of audio files it uses the following
 * functions to achieve this.
 *
 * http://sdl.beuc.net/sdl.wiki/Mix_GetError
 * http://sdl.beuc.net/sdl.wiki/Mix_LoadWAV
 * http://sdl.beuc.net/sdl.wiki/Mix_PlayChannel
 * http://sdl.beuc.net/sdl.wiki/Mix_PlayingMusic
 * http://sdl.beuc.net/sdl.wiki/Mix_PausedMusic
 * http://sdl.beuc.net/sdl.wiki/Mix_ResumeMusic
 * http://sdl.beuc.net/sdl.wiki/Mix_PauseMusic
 */
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_mixer.h>
#include <stdio.h>

const int SCREEN_WIDTH = 640;
const int SCREEN_HEIGHT = 480;

typedef struct {
	SDL_Texture *mTexture;
	int mWidth;
	int mHeight;
} LTexture;

SDL_Window* gWindow = NULL;
SDL_Renderer* gRenderer = NULL;
LTexture gPromptTexture;
Mix_Music *gMusic = NULL;

Mix_Chunk *gScratch = NULL;
Mix_Chunk *gHigh = NULL;
Mix_Chunk *gMedium = NULL;
Mix_Chunk *gLow = NULL;

short init()
{
	if(SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO) < 0) {
		SDL_Log("%s(), SDL_Init failed.", __func__);
		return -1;
	}

	if(SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "2") == 0)
		SDL_Log("%s(), Warning: Linear texture filtering disabled",
				__func__);

	gWindow = SDL_CreateWindow(
			"SDL Tutorial",
			SDL_WINDOWPOS_UNDEFINED,
			SDL_WINDOWPOS_UNDEFINED,
			SCREEN_WIDTH,SCREEN_HEIGHT,
			SDL_WINDOW_SHOWN);

	if(gWindow == NULL) {
		SDL_Log("%s(), SDL_CreateWindow failed.", __func__);
		return -1;
	}

	gRenderer = SDL_CreateRenderer(gWindow, -1, SDL_RENDERER_ACCELERATED);
	if(gRenderer == NULL) {
		SDL_Log("%s(), SDL_CreateRenderer failed.", __func__);
		return -1;
	}

	SDL_SetRenderDrawColor(gRenderer, 0xFF, 0xFF, 0xFF, 0xFF);

	if((IMG_Init(IMG_INIT_PNG) & IMG_INIT_PNG) == 0) {
		SDL_Log("%s(), IMG_Init failed.", __func__);
		return -1;
	}

	if(Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048) < 0) {
		SDL_Log("%s(), Mix_OpenAudioSDL_mixer could not initiled. %s",
				__func__, Mix_GetError());
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

short LTexture_loadFromFile(LTexture *lt, char *path)
{
	free_texture(lt);

	SDL_Texture* newTexture = NULL;

	SDL_Surface* loadedSurface = IMG_Load(path);
	if(loadedSurface == NULL) {
		SDL_Log("%s(), IMG_Load failed to load \"%s\".",
				__func__, path);
		return -1;
	}

	SDL_SetColorKey(
			loadedSurface,
			SDL_TRUE,
			SDL_MapRGB(loadedSurface->format, 0, 0xFF, 0xFF));

	newTexture = SDL_CreateTextureFromSurface(gRenderer, loadedSurface);
	if(newTexture == NULL) {
		SDL_Log("%s(), SDL_CreateTextureFromSurface failed.",
				__func__);
		return -1;
	}

	lt->mWidth = loadedSurface->w;
	lt->mHeight = loadedSurface->h;

	SDL_FreeSurface(loadedSurface);

	lt->mTexture = newTexture;

	return 0;
}

short LTexture_render(LTexture *lt)
{
	return SDL_RenderCopy(gRenderer, lt->mTexture, NULL, NULL);
}

short loadMedia()
{
	if(LTexture_loadFromFile(&gPromptTexture, "prompt.png"))
		return -1;

	gMusic = Mix_LoadMUS("beat.wav");
	if(gMusic == NULL) {
		SDL_Log( "%s(), Mix_LoadMUS failed.", __func__);
		return -1;
	}
	
	gScratch = Mix_LoadWAV("scratch.wav");
	if(gScratch == NULL) {
		SDL_Log( "%s(), Mix_LoadWAV failed.", __func__);
		return -1;
	}
	
	gHigh = Mix_LoadWAV("high.wav");
	if(gHigh == NULL) {
		SDL_Log( "%s(), Mix_LoadWAV failed.", __func__);
		return -1;
	}

	gMedium = Mix_LoadWAV("medium.wav");
	if(gMedium == NULL) {
		SDL_Log( "%s(), Mix_LoadWAV failed.", __func__);
		return -1;
	}

	gLow = Mix_LoadWAV("low.wav");
	if(gLow == NULL) {
		SDL_Log( "%s(), Mix_LoadWAV failed.", __func__);
		return -1;
	}

	return 0;
}

void close_all()
{
	free_texture(&gPromptTexture);

	Mix_FreeChunk(gScratch);
	Mix_FreeChunk(gHigh);
	Mix_FreeChunk(gMedium);
	Mix_FreeChunk(gLow);
	gScratch = NULL;
	gHigh = NULL;
	gMedium = NULL;
	gLow = NULL;
	
	Mix_FreeMusic(gMusic);
	gMusic = NULL;

	SDL_DestroyRenderer(gRenderer);
	SDL_DestroyWindow(gWindow);
	gWindow = NULL;
	gRenderer = NULL;

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
	SDL_Event e;

	if(init())
		goto equit;

	if(loadMedia())
		goto equit;

	while(1)
	{
		while(SDL_PollEvent(&e) != 0)
		{
			if(e.type == SDL_QUIT)
				goto equit;

			if(e.type == SDL_KEYDOWN)
				get_key_pressed(&e);
		}

		SDL_SetRenderDrawColor(gRenderer, 0xFF, 0xFF, 0xFF, 0xFF);
		SDL_RenderClear(gRenderer);

		LTexture_render(&gPromptTexture);

		SDL_RenderPresent(gRenderer);
		SDL_Delay(16);
	}
equit:
	close_all();

	return 0;
}

