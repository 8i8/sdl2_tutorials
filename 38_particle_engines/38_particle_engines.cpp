/*This source code copyrighted by Lazy Foo' Productions (2004-2015)
and may not be redistributed without written permission.*/

//Using SDL, SDL_image, standard IO, and strings
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <stdio.h>
#include <string>

//Screen dimension constants
const int SCREEN_WIDTH = 640;
const int SCREEN_HEIGHT = 480;

//Particle count
const int TOTAL_PARTICLES = 20;

//Texture wrapper class
class LTexture
{
	public:
		//Initializes variables
		LTexture();

		//Deallocates memory
		~LTexture();

		//Loads image at specified path
		bool loadFromFile(std::string path);
		
		#ifdef _SDL_TTF_H
		//Creates image from font string
		bool loadFromRenderedText(
				std::string textureText,
				SDL_Color textColor);
		#endif

		//Deallocates texture
		void free();

		//Set color modulation
		void setColor(Uint8 red, Uint8 green, Uint8 blue);

		//Set blending
		void setBlendMode(SDL_BlendMode blending);

		//Set alpha modulation
		void setAlpha(Uint8 alpha);
		
		//Renders texture at given point
		void render(
				int x,
				int y,
				SDL_Rect* clip = NULL,
				double angle = 0.0,
				SDL_Point* center = NULL,
				SDL_RendererFlip flip = SDL_FLIP_NONE);

		//Gets image dimensions
		int getWidth();
		int getHeight();

	private:
		//The actual hardware texture
		SDL_Texture* mTexture;

		//Image dimensions
		int mWidth;
		int mHeight;
};

class Particle
{
	public:
		//Initialize position and animation
		Particle(int x, int y);

		//Shows the particle
		void render();

		//Checks if particle is dead
		bool isDead();

	private:
		//Offsets
		int mPosX, mPosY;

		//Current frame of animation
		int mFrame;

		//Type of particle
		LTexture *mTexture;
};


//The dot that will move around on the screen
class Dot
{
	public:
		//The dimensions of the dot
		static const int DOT_WIDTH = 20;
		static const int DOT_HEIGHT = 20;

		//Maximum axis velocity of the dot
		static const int DOT_VEL = 10;

		//Initializes the variables and allocates particles
		Dot();

		//Deallocates particles
		~Dot();

		//Takes key presses and adjusts the dot's velocity
		void handleEvent(SDL_Event& e);

		//Moves the dot
		void move();

		//Shows the dot on the screen
		void render();

	private:
		//The particles
		Particle* particles[TOTAL_PARTICLES];

		//Shows the particles
		void renderParticles();

		//The X and Y offsets of the dot
		int mPosX, mPosY;

		//The velocity of the dot
		int mVelX, mVelY;
};


SDL_Window* gWindow = NULL;		//The window we'll be rendering to
SDL_Renderer* gRenderer = NULL;		//The window renderer
LTexture gDotTexture;			//Scene textures
LTexture gRedTexture;
LTexture gGreenTexture;
LTexture gBlueTexture;
LTexture gShimmerTexture;

LTexture::LTexture()
{
	//Initialize
	mTexture = NULL;
	mWidth = 0;
	mHeight = 0;
}

LTexture::~LTexture()
{
	//Deallocate
	free();
}

bool LTexture::loadFromFile(std::string path)
{
	//Get rid of preexisting texture
	free();

	//The final texture
	SDL_Texture* newTexture = NULL;

	//Load image at specified path
	SDL_Surface* loadedSurface = IMG_Load(path.c_str());
	if(loadedSurface == NULL) {
		printf("Unable to load image %s! SDL_image Error: %s\n",
				path.c_str(), IMG_GetError());
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
				path.c_str(), SDL_GetError() );
		return -1;
	}

	//Get image dimensions
	mWidth = loadedSurface->w;
	mHeight = loadedSurface->h;

	//Get rid of old loaded surface
	SDL_FreeSurface( loadedSurface );

	//Return success
	mTexture = newTexture;

	return 0;
}

#ifdef _SDL_TTF_H
bool LTexture::loadFromRenderedText(
				std::string textureText,
				SDL_Color textColor)
{
	//Get rid of preexisting texture
	free();

	//Render text surface
	SDL_Surface* textSurface = TTF_RenderText_Solid(
						gFont,
						textureText.c_str(),
						textColor);
	if(textSurface == NULL) {
		printf("Unable to render text surface! SDL_ttf Error: %s\n",
				TTF_GetError());
		return -1;
	}

	//Create texture from surface pixels
	mTexture = SDL_CreateTextureFromSurface(gRenderer, textSurface);
	if(mTexture == NULL) {
		printf("Unable to create texture from rendered text! SDL Error: %s\n",
				SDL_GetError());
		return -1;
	}

	//Get image dimensions
	mWidth = textSurface->w;
	mHeight = textSurface->h;

	//Get rid of old surface
	SDL_FreeSurface( textSurface );
	
	//Return success
	return 0;
}
#endif

void LTexture::free()
{
	//Free texture if it exists
	if(mTexture != NULL) {
		SDL_DestroyTexture(mTexture);
		mTexture = NULL;
		mWidth = 0;
		mHeight = 0;
	}
}

void LTexture::setColor(Uint8 red, Uint8 green, Uint8 blue)
{
	//Modulate texture rgb
	SDL_SetTextureColorMod(mTexture, red, green, blue);
}

void LTexture::setBlendMode(SDL_BlendMode blending)
{
	//Set blending function
	SDL_SetTextureBlendMode(mTexture, blending);
}
		
void LTexture::setAlpha(Uint8 alpha)
{
	//Modulate texture alpha
	SDL_SetTextureAlphaMod( mTexture, alpha );
}

void LTexture::render(
			int x,
			int y,
			SDL_Rect* clip,
			double angle,
			SDL_Point* center,
			SDL_RendererFlip flip)
{
	//Set rendering space and render to screen
	SDL_Rect renderQuad = { x, y, mWidth, mHeight };

	//Set clip rendering dimensions
	if( clip != NULL )
	{
		renderQuad.w = clip->w;
		renderQuad.h = clip->h;
	}

	//Render to screen
	SDL_RenderCopyEx(
			gRenderer,
			mTexture,
			clip,
			&renderQuad,
			angle,
			center,
			flip);
}

int LTexture::getWidth()	
{
	return mWidth;
}

int LTexture::getHeight()
{
	return mHeight;
}

Particle::Particle(int x, int y)
{
	//Seed random
	srand(SDL_GetTicks());

	//Set offsets
	mPosX = x - 5 + (rand() % 25);
	mPosY = y - 5 + (rand() % 25);

	//Initialize animation
	mFrame = rand() % 5;

	//Set type
	switch(rand() % 3)
	{
		case 0: mTexture = &gRedTexture; break;
		case 1: mTexture = &gGreenTexture; break;
		case 2: mTexture = &gBlueTexture; break;
	}
}

void Particle::render()
{
	//Show image
	mTexture->render(mPosX, mPosY);

	//Show shimmer
	if(mFrame % 2 == 0)
		gShimmerTexture.render(mPosX, mPosY);

	//Animate
	mFrame++;
}

bool Particle::isDead()
{
	return mFrame > 10;
}

Dot::Dot()
{
	int i;
	//Initialize the offsets
	mPosX = 0;
	mPosY = 0;

	//Initialize the velocity
	mVelX = 0;
	mVelY = 0;

	//Initialize particles
	for(i = 0; i < TOTAL_PARTICLES; ++i)
		particles[i] = new Particle(mPosX, mPosY);
}

Dot::~Dot()
{
	int i;
	//Delete particles
	for(i = 0; i < TOTAL_PARTICLES; ++i)
		delete particles[i];
}

void Dot::handleEvent(SDL_Event& e)
{
	//If a key was pressed
	if(e.type == SDL_KEYDOWN && e.key.repeat == 0)
	{
		//Adjust the velocity
		switch(e.key.keysym.sym) {
			case SDLK_UP: mVelY -= DOT_VEL; break;
			case SDLK_DOWN: mVelY += DOT_VEL; break;
			case SDLK_LEFT: mVelX -= DOT_VEL; break;
			case SDLK_RIGHT: mVelX += DOT_VEL; break;
		}
	}
	//If a key was released
	else if(e.type == SDL_KEYUP && e.key.repeat == 0)
	{
		//Adjust the velocity
		switch(e.key.keysym.sym) {
			case SDLK_UP: mVelY += DOT_VEL; break;
			case SDLK_DOWN: mVelY -= DOT_VEL; break;
			case SDLK_LEFT: mVelX += DOT_VEL; break;
			case SDLK_RIGHT: mVelX -= DOT_VEL; break;
		}
	}
}

void Dot::move()
{
	//Move the dot left or right
	mPosX += mVelX;

	//If the dot went too far to the left or right
	if((mPosX < 0) || (mPosX + DOT_WIDTH > SCREEN_WIDTH))
		mPosX -= mVelX;

	//Move the dot up or down
	mPosY += mVelY;

	//If the dot went too far up or down
	if((mPosY < 0) || (mPosY + DOT_HEIGHT > SCREEN_HEIGHT))
		mPosY -= mVelY;
}

void Dot::render()
{
	//Show the dot
	gDotTexture.render(mPosX, mPosY);

	//Show particles on top of dot
	renderParticles();
}

void Dot::renderParticles()
{
	int i;
	//Go through particles
	for(i = 0; i < TOTAL_PARTICLES; ++i)
		//Delete and replace dead particles
		if(particles[i]->isDead()) {
			delete particles[i];
			particles[i] = new Particle(mPosX, mPosY);
		}

	//Show particles
	for(i = 0; i < TOTAL_PARTICLES; ++i)
		particles[i]->render();
}

bool init()
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

	//Create renderer for window
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
		printf("SDL_image could not initialize! SDL_image Error: %s\n",
				IMG_GetError());
		return -1;
	}

	return 0;
}

bool loadMedia()
{
	//Load dot texture
	if(gDotTexture.loadFromFile("dot.bmp") < 0) {
		printf("Failed to load dot texture!\n");
		return -1;
	}

	//Load red texture
	if(gRedTexture.loadFromFile("red.bmp") < 0) {
		printf("Failed to load red texture!\n");
		return -1;
	}

	//Load green texture
	if(gGreenTexture.loadFromFile("green.bmp") < 0) {
		printf( "Failed to load green texture!\n" );
		return -1;
	}

	//Load blue texture
	if(gBlueTexture.loadFromFile("blue.bmp") < 0) {
		printf( "Failed to load blue texture!\n" );
		return -1;
	}

	//Load shimmer texture
	if(gShimmerTexture.loadFromFile("shimmer.bmp") < 0) {
		printf("Failed to load shimmer texture!\n");
		return -1;
	}
	
	//Set texture transparency
	gRedTexture.setAlpha( 192 );
	gGreenTexture.setAlpha( 192 );
	gBlueTexture.setAlpha( 192 );
	gShimmerTexture.setAlpha( 192 );

	return 0;
}

void close_all()
{
	//Free loaded images
	gDotTexture.free();
	gRedTexture.free();
	gGreenTexture.free();
	gBlueTexture.free();
	gShimmerTexture.free();

	//Destroy window	
	SDL_DestroyRenderer(gRenderer);
	SDL_DestroyWindow(gWindow);
	gWindow = NULL;
	gRenderer = NULL;

	//Quit SDL subsystems
	IMG_Quit();
	SDL_Quit();
}

int main(int argc, char* argv[])
{
	SDL_Event e;
	Dot dot;

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
		while(SDL_PollEvent(&e) != 0) {
			if(e.type == SDL_QUIT)
				goto equit;

			//Handle input for the dot
			dot.handleEvent(e);
		}

		//Move the dot
		dot.move();

		//Clear screen
		SDL_SetRenderDrawColor(gRenderer, 0xFF, 0xFF, 0xFF, 0xFF);
		SDL_RenderClear(gRenderer);

		//Render objects
		dot.render();

		//Update screen
		SDL_RenderPresent(gRenderer);
	}
equit:
	//Free resources and close SDL
	close_all();

	return 0;
}
