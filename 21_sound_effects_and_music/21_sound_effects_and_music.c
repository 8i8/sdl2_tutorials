/*
 * Sound Effects and Music
 *
 * Up until now we've only been dealing with video and input. Most games made
 * require some sort of sound and here we'll be using SDL_mixer to play audio
 * for us.
 *
 * SDL_mixer is a library we use to make audio playing easier (because it can
 * get complicated). We have to set it up just like we set up SDL_image. Like
 * before, it's just a matter of having the headers files, library files, and
 * binary files in the right place with your compiler configured to use them.
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

/*
 * The SDL_mixer data type for music is Mix_Music and one short sounds is
 * Mix_Chunk. Here we declare pointers for the music and sound effects we'll be
 * using.
 */
Mix_Music *gMusic = NULL;
Mix_Chunk *gScratch = NULL;
Mix_Chunk *gHigh = NULL;
Mix_Chunk *gMedium = NULL;
Mix_Chunk *gLow = NULL;

/*
 * Since we're using music and sound effects, we need to initialize audio along
 * with video for this demo.
 */
short init(void)
{
	if(SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO) < 0) {
		SDL_Log("%s(), SDL_Init failed. %s", __func__, SDL_GetError());
		return -1;
	}

	if(SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "2") == 0)
		SDL_Log("Warning: Linear texture filtering disabled.");


	gWindow = SDL_CreateWindow(
			"SDL Tutorial",
			SDL_WINDOWPOS_UNDEFINED,
			SDL_WINDOWPOS_UNDEFINED,
			SCREEN_WIDTH,SCREEN_HEIGHT,
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
/*
 * To initialize SDL_mixer we need to call Mix_OpenAudio. The first argument
 * sets the sound frequency, and 44100 is a standard frequency that works on
 * most systems. The second argument determines the sample format, which again
 * here we're using the default. The third argument is the number of hardware
 * channels, and here we're using 2 channels for stereo. The last argument is
 * the sample size, which determines the size of the chunks we use when playing
 * sound. 2048 bytes (AKA 2 kilobyes) worked fine for me, but you may have to
 * experiment with this value to minimize lag when playing sounds.
 *
 * If there's any errors with SDL_mixer, they're reported with Mix_GetError.
 */
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
		SDL_Log("%s(), IMG_Load failed. %s", __func__, IMG_GetError());

		return -1;
	}

	SDL_SetColorKey(
			loadedSurface,
			SDL_TRUE,
			SDL_MapRGB(loadedSurface->format, 0, 0xFF, 0xFF));

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

short LTexture_render(LTexture *lt)
{
	return SDL_RenderCopy(gRenderer, lt->mTexture, NULL, NULL);
}

/*
 * Here we load our splash texture and sound.
 *
 * To load music we call Mix_LoadMUS and to load sound effect we call
 * Mix_LoadWAV.
 */
short loadMedia(void)
{
	if(LTexture_loadFromFile(&gPromptTexture, "prompt.png"))
		return -1;

	gMusic = Mix_LoadMUS("beat.wav");
	if(gMusic == NULL) {
		SDL_Log( "%s(), Mix_LoadMUS failed. %s", __func__, Mix_GetError());
		return -1;
	}

	gScratch = Mix_LoadWAV("scratch.wav");
	if(gScratch == NULL) {
		SDL_Log( "%s(), Mix_LoadWAV failed. %s", __func__, Mix_GetError());
		return -1;
	}

	gHigh = Mix_LoadWAV("high.wav");
	if(gHigh == NULL) {
		SDL_Log( "%s(), Mix_LoadWAV failed. %s", __func__, Mix_GetError());
		return -1;
	}

	gMedium = Mix_LoadWAV("medium.wav");
	if(gMedium == NULL) {
		SDL_Log( "%s(), Mix_LoadWAV failed. %s", __func__, Mix_GetError());
		return -1;
	}

	gLow = Mix_LoadWAV("low.wav");
	if(gLow == NULL) {
		SDL_Log( "%s(), Mix_LoadWAV failed. %s", __func__, Mix_GetError());
		return -1;
	}

	return 0;
}

/*
 * When we're done with audio and want to free it, we call Mix_FreeMusic to
 * free music and Mix_FreeChunk to free a sound effect. We call Mix_Quit to
 * close down SDL_mixer.
 */
void close_all(void)
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

/*
 * In the event loop, we play a sound effect when the 1, 2, 3, or 4 keys are
 * pressed. The way to play a Mix_Chunk is by calling Mix_PlayChannel. The
 * first argument is the channel you want to use to play it. Since we don't
 * care which channel it comes out of, we set the channel to negative 1 which
 * will use the nearest available channel. The second argument is the sound
 * effect and last argument is the number of times to repeat the effect. We
 * only want it to play once per button press, so we have it repeat 0 times.
 *
 * A channel in this case is not the same as a hardware channel that can
 * represent the left and right channel of a stereo system. Every sound effect
 * that's played has a channel associated with it. When you want to pause or
 * stop an effect that is play, you can halt its channel.
 */
/*
 * For this demo, we want to play/pause the music on a 9 keypress and stop the
 * music on a 0 keypress.
 *
 * When the 9 key pressed we first check if the music is not playing with
 * Mix_PlayingMusic. If it isn't, we start the music with Mix_PlayMusic. The
 * first argument is the music we want to play and the last argument is the
 * number of times to repeat it. Negative 1 is a special value saying we want
 * to loop it until it is stopped.
 *
 * If there is music playing, we check if the music is paused using
 * Mix_PausedMusic. If the music is paused, we resume it using Mix_ResumeMusic.
 * If the music is not paused we pause it using Mix_PauseMusic.
 *
 * When 0 is pressed, we stop music if it playing using Mix_HaltMusic.
 */
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
			if(Mix_PlayingMusic() == 0)
				Mix_PlayMusic(gMusic, -1);
			else if(Mix_PausedMusic() == 1)
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

int main(void)
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

