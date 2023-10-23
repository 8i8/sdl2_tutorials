/*
 * File Reading and Writing
 *
 * Being able to save and load data is needed to keep data between play
 * sessions. SDL_RWops file handling allows us to do cross platform file IO to
 * save data.
 */
#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>

#define SCREEN_WIDTH	640
#define SCREEN_HEIGHT	480
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

SDL_Window* gWindow = NULL;
SDL_Renderer* gRenderer = NULL;
TTF_Font *gFont = NULL;
LTexture gPromptTextTexture;
LTexture gDataTextures[TOTAL_DATA];
STdata gStr;

/*
 * Here we're declaring an array of Signed integers that are 32 bits long. This
 * will be the data we will be loading and saving. For this demo this array
 * will be of length 10.
 */
Sint32 gData[TOTAL_DATA];

short init(void)
{
	if(SDL_Init(SDL_INIT_VIDEO) < 0) {
		SDL_Log("%s(), SDL_Init failed. %s", __func__, SDL_GetError());
		return -1;
	}

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

	if(TTF_Init() == -1) {
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

short LTexture_render(LTexture *lt, int x, int y, SDL_Rect* clip)
{
	SDL_Rect renderQuad = {x, y, lt->mWidth, lt->mHeight};

	if(clip != NULL) {
		renderQuad.w = clip->w;
		renderQuad.h = clip->h;
	}

	return SDL_RenderCopy(gRenderer, lt->mTexture, clip, &renderQuad);
}

void gStr_init(STdata *gStr)
{
	SDL_Color textColor = { 0, 0, 0, 0xFF };
	SDL_Color highlightColor = { 0xFF, 0, 0, 0xFF };

	gStr->textColor = textColor;
	gStr->highlightColor = highlightColor;
	gStr->currentData = 0;
}

/*
 * In our media loading function we're opening the save file for reading using
 * SDL_RWFromFile. The first argument is the path to the file and the second
 * argument defines how we will be opening it. "r+b" means it is being opened
 * for reading in binary.
 */
short loadMedia(void)
{
	int i;
	char string[255];

	gStr_init(&gStr);

	gFont = TTF_OpenFont("lazy.ttf", 28);
	if(gFont == NULL) {
		SDL_Log("%s(), TTF_OpenFont failed. %s", __func__, TTF_GetError());
		return -1;
	}

	if(LTexture_loadFromRenderedText(
				&gPromptTextTexture,
				"Enter Data:",
				gStr.textColor) < 0)
		return -1;

	SDL_RWops* file = SDL_RWFromFile("nums.bin", "r+b");
/*
 * If the file does not exist, try to create it using "w+b" the write binary
 * mode, if that fails return -1. If a new file is made write a message to the
 * command line. Write '0' to every position in the gData array, and then close
 * the file handler. If the file was opened read in the data from each line.
 */
	if(file == NULL)
	{
		if((file = SDL_RWFromFile("nums.bin", "w+b")) == NULL) {
			SDL_Log("%s(), SDL_RWFromFile failed.", __func__);
			return -1;
		}
		SDL_Log("New file created.");

		for(i = 0; i < TOTAL_DATA; ++i) {
			gData[i] = 0;	
			SDL_RWwrite(file, &gData[i], sizeof(Sint32), 1);
		}
		
		SDL_RWclose(file);
	}
	else
	{
		SDL_Log("Reading file...");
		for(i = 0; i < TOTAL_DATA; ++i)
			SDL_RWread(file, &gData[i], sizeof(Sint32), 1);

		SDL_RWclose(file);
	}
/*
 * After the file is loaded we render the text textures that correspond with
 * each of our data numbers. Our loadFromRenderedText function only accepts
 * strings so we have to convert the integers to strings.
 */
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

/*
 *  When we close the program, we open up the file again for writing and write
 *  out all the data.
 */
void close_all(void)
{
	int i;
	SDL_RWops* file = SDL_RWFromFile("nums.bin", "w+b");
	if(file != NULL) {
		SDL_Log("Writing file...");
		for(i = 0; i < TOTAL_DATA; ++i)
			SDL_RWwrite(file, &gData[i], sizeof(Sint32), 1);

		SDL_RWclose(file);
	}
	else
		SDL_Log("%s(), file save failed. %s\n", __func__,
				SDL_GetError());

	free_texture(&gPromptTextTexture);
	for(i = 0; i < TOTAL_DATA; ++i)
		free_texture(&gDataTextures[i]);

	TTF_CloseFont(gFont);
	gFont = NULL;

	SDL_DestroyRenderer(gRenderer);
	SDL_DestroyWindow(gWindow);
	gWindow = NULL;
	gRenderer = NULL;

	TTF_Quit();
	SDL_Quit();
}

/*
 * When we press up or down we want to re-render the old current data in plain
 * colour, move to the next data point (with some bounds checking), and
 * re-render the new current data in the highlight colour. When we press left
 * or right we decrement or increment the current data and re-render the
 * texture associated with it. 
 */
void get_key(SDL_Event *e, STdata *str)
{
	char string[255];
	switch(e->key.keysym.sym)
	{
		case SDLK_UP:
			sprintf(string, "%d", gData[str->currentData]);
			LTexture_loadFromRenderedText(
					&gDataTextures[str->currentData],
					string,
					str->textColor);
			--(str->currentData);
			if(str->currentData < 0)
				str->currentData = TOTAL_DATA - 1;
			
			sprintf(string, "%d", gData[str->currentData]);
			LTexture_loadFromRenderedText(
					&gDataTextures[str->currentData],
					string,
					str->highlightColor);
			break;
		
		case SDLK_DOWN:
			sprintf(string, "%d", gData[str->currentData]);
			LTexture_loadFromRenderedText(
					&gDataTextures[str->currentData],
					string,		
					str->textColor);
			++(str->currentData);
			if(str->currentData == TOTAL_DATA)
				str->currentData = 0;
			
			sprintf(string, "%d", gData[str->currentData]);
			LTexture_loadFromRenderedText(
					&gDataTextures[str->currentData],
					string,
					str->highlightColor);
			break;

		case SDLK_LEFT:
			--gData[str->currentData];
			sprintf(string, "%d", gData[str->currentData]);
			LTexture_loadFromRenderedText(
					&gDataTextures[str->currentData],
					string,
					str->highlightColor);
			break;
		
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

/*
 * Before we go into the main loop we declare currentData to keep track of
 * which of our data integers we're altering. We also declare a plain text
 * colour and a highlight colour for rendering text.
 */
int main(int argc, char* args[])
{
	int i;

	if(init())
		goto equit;

	if(loadMedia())
		goto equit;

	SDL_Event e;

	while(1)
	{
		while(SDL_PollEvent(&e) != 0) {
			if(e.type == SDL_QUIT)
				goto equit;

			if(e.type == SDL_KEYDOWN)
				get_key(&e, &gStr);
		}

		SDL_SetRenderDrawColor(gRenderer, 0xFF, 0xFF, 0xFF, 0xFF);
		SDL_RenderClear(gRenderer);

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

		SDL_RenderPresent(gRenderer);
	}
equit:
	close_all();

	return 0;
}
