/*
 * Per-pixel Collision Detection
 *
 * Once you know how to check collision between two rectangles, you can check
 * collision between any two images since all images are made out of
 * rectangles.
 *
 * TODO Note that this implimentation will only work when the DOT_VEL is set to
 * one, if it is set any higher ther is a space between the two dots when a
 * collision is detected.
 */
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>

#define ZONES			11
#define SCREEN_WIDTH	640
#define SCREEN_HEIGHT	480
#define DOT_WIDTH		20
#define DOT_HEIGHT		20
#define DOT_VEL			1
#define JOYSTICK_DEAD_ZONE	8000

typedef struct {
	SDL_Texture *mTexture;
	int mWidth;
	int mHeight;
} LTexture;

/*
 * Everything can be made out of rectangles in a video game, even this dot.
 * Images are made out of pixels which are squares which are rectangles. To do
 * per-pixel collision detection all we have to do is have each object have a
 * set of box colliders and check collision of one set of collision boxes
 * against another.
 *
 * Here is our dot now with per-pixel collision detection. Its velocity is
 * reduced to 1 pixel per frame to make the collision easier to see. The move
 * function now takes in a vector of collision boxes so we can check two sets
 * against each other. Since we're going to have two dots colliding, we need to
 * be able to get the colliders so we have a function for that.
 *
 * Instead of having a single collision box, we have a vector of colliders. We
 * also have a related function to shift the colliders to match the position of
 * the dot.
 *
 * We also have our new collision detector that checks sets of collision boxes
 * against each other.
 */
typedef struct {
	int mPosX, mPosY;
	int mVelX, mVelY;
	SDL_Rect mColliders[ZONES];
} Dot;

short checkCollision(SDL_Rect *a, SDL_Rect *b);
void Dot_shiftColliders(Dot *d);

SDL_Window* gWindow = NULL;
SDL_Renderer* gRenderer = NULL;
SDL_GameController* gGameController = NULL;
LTexture gDotTexture;

short init(void)
{
	if(SDL_Init(SDL_INIT_VIDEO | SDL_INIT_GAMECONTROLLER) < 0) {
		SDL_Log("%s(), SDL_Init failed. %s", __func__, SDL_GetError());
		return -1;
	}

    for (int i = 0; i < SDL_NumJoysticks(); i++) {
        if (SDL_IsGameController(i)) {
            gGameController = SDL_GameControllerOpen(i);
			if(gGameController == NULL) {
				SDL_Log("%s(), SDL_JoystickOpen failed. %s", __func__, SDL_GetError());
			}
			break;
        }
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
					SDL_RENDERER_ACCELERATED);
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

short LTexture_render(LTexture *lt, int x, int y, SDL_Rect* clip)
{
	SDL_Rect renderQuad = {x, y, lt->mWidth, lt->mHeight};

	if(clip != NULL) {
		renderQuad.w = clip->w;
		renderQuad.h = clip->h;
	}

	return SDL_RenderCopy(gRenderer, lt->mTexture, clip, &renderQuad);
}

/*
 * Just like before, we have to set the collider dimensions in the constructor.
 * Only difference here is that we have multiple collision boxes to set.
 */
void Dot_init(Dot *d, int x, int y)
{
	d->mPosX = x;
	d->mPosY = y;

	d->mVelX = 0;
	d->mVelY = 0;

	d->mColliders[0].w = 6;
	d->mColliders[0].h = 1;
	d->mColliders[1].w = 10;
	d->mColliders[1].h = 1;
	d->mColliders[2].w = 14;
	d->mColliders[2].h = 1;
	d->mColliders[3].w = 16;
	d->mColliders[3].h = 2;
	d->mColliders[4].w = 18;
	d->mColliders[4].h = 2;
	d->mColliders[5].w = 20;
	d->mColliders[5].h = 6;
	d->mColliders[6].w = 18;
	d->mColliders[6].h = 2;
	d->mColliders[7].w = 16;
	d->mColliders[7].h = 2;
	d->mColliders[8].w = 14;
	d->mColliders[8].h = 1;
	d->mColliders[9].w = 10;
	d->mColliders[9].h = 1;
	d->mColliders[10].w = 6;
	d->mColliders[10].h = 1;

	Dot_shiftColliders(d);
}

void handle_keyboard_events(Dot *d, SDL_Event *e)
{
	if(e->type == SDL_KEYDOWN && e->key.repeat == 0)
	{
		switch(e->key.keysym.sym)
		{
			case SDLK_UP: 	d->mVelY -= DOT_VEL; break;
			case SDLK_DOWN: d->mVelY += DOT_VEL; break;
			case SDLK_LEFT: d->mVelX -= DOT_VEL; break;
			case SDLK_RIGHT:d->mVelX += DOT_VEL; break;
		}
	}
	else if(e->type == SDL_KEYUP && e->key.repeat == 0)
	{
		switch(e->key.keysym.sym)
		{
			case SDLK_UP:	d->mVelY += DOT_VEL; break;
			case SDLK_DOWN: d->mVelY -= DOT_VEL; break;
			case SDLK_LEFT: d->mVelX += DOT_VEL; break;
			case SDLK_RIGHT:d->mVelX -= DOT_VEL; break;
		}
	}
	else if(e->type == SDL_JOYAXISMOTION)
	{
		if(e->jaxis.which == 0) {
			if(e->jaxis.axis == 0)
			{
				if(e->jaxis.value < -JOYSTICK_DEAD_ZONE)
					d->mVelX -= DOT_VEL;
				else if(e->jaxis.value > JOYSTICK_DEAD_ZONE)
					d->mVelX += DOT_VEL;
				else
					d->mVelX = 0;
			}
			else if(e->jaxis.axis == 1)
			{
				if(e->jaxis.value < -JOYSTICK_DEAD_ZONE)
					d->mVelY -= DOT_VEL;
				else if(e->jaxis.value > JOYSTICK_DEAD_ZONE)
					d->mVelY += DOT_VEL;
				else
					d->mVelY = 0;
			}
		}
	}
}

void handle_axis_motion(SDL_Event *e, Dot *d) {
	if(e->jaxis.which == 0) {
		if(e->jaxis.axis == 0)
		{
			if(e->jaxis.value < -JOYSTICK_DEAD_ZONE)
				d->mVelX -= DOT_VEL;
			else if(e->jaxis.value > JOYSTICK_DEAD_ZONE)
				d->mVelX += DOT_VEL;
			else
				d->mVelX = 0;
		}
		else if(e->jaxis.axis == 1)
		{
			if(e->jaxis.value < -JOYSTICK_DEAD_ZONE)
				d->mVelY -= DOT_VEL;
			else if(e->jaxis.value > JOYSTICK_DEAD_ZONE)
				d->mVelY += DOT_VEL;
			else
				d->mVelY = 0;
		}
	}
}

/*
 * This functions pretty much the same as before. Whenever we move the dot, we
 * move the collider(s) with it. After we move the dot, we check if it went off
 * screen or hit something. If it did, we move the dot back and move its
 * colliders with it.
 *
 * n't worry too much about how shiftColliders works. It's a short hand way of
 * mColliders[ 0 ].x = ..., mColliders[ 1 ].x = ..., etc and it works for this
 * specific case. For your own per-pixel objects you'll have your own placing
 * functions.
 *
 * And after shiftColliders, have an accessor function to get the colliders.
 */
void Dot_move(Dot *d, SDL_Rect *otherColliders)
{
	d->mPosX += d->mVelX;
	Dot_shiftColliders(d);

	if(
			(d->mPosX < 0)
			|| (d->mPosX + DOT_WIDTH > SCREEN_WIDTH)
			|| checkCollision(d->mColliders, otherColliders))
	{
		d->mPosX -= d->mVelX;
		Dot_shiftColliders(d);
	}

	d->mPosY += d->mVelY;
	Dot_shiftColliders(d);

	if(
			(d->mPosY < 0)
			|| (d->mPosY + DOT_HEIGHT > SCREEN_HEIGHT)
			|| checkCollision(d->mColliders, otherColliders))
	{
		d->mPosY -= d->mVelY;
		Dot_shiftColliders(d);
	}
}

void Dot_render(Dot *d)
{
	LTexture_render(&gDotTexture, d->mPosX, d->mPosY, NULL);
}

/*
 * Don't worry too much about how shiftColliders works. It's a short hand way
 * of mColliders[ 0 ].x = ..., mColliders[ 1 ].x = ..., etc and it works for
 * this specific case. For your own per-pixel objects you'll have your own
 * placing functions.
 *
 * And after shiftColliders, have an accessor function to get the colliders.
 */
void Dot_shiftColliders(Dot *d)
{
	int zone, r;
	r = 0;

	for(zone = 0; zone < ZONES; ++zone)
	{
		d->mColliders[zone].x =
			d->mPosX + (DOT_WIDTH - d->mColliders[zone].w) / 2;

		d->mColliders[zone].y = d->mPosY + r;

		r += d->mColliders[zone].h;
	}
}

/*
 * Here in our collision detection function, we have a for loop that calculates
 * the top/bottom/left/right of each collision box in object a. 
 *
 * We then check calculate the top/bottom/left/right of each collision box in
 * object b. We then check if there is no separating axis. If this no
 * separating axis, we return true. If we get through both sets without a
 * collision, we return false.
 */
short checkCollision(SDL_Rect *a, SDL_Rect *b)
{
	int leftA, leftB;
	int rightA, rightB;
	int topA, topB;
	int bottomA, bottomB;
	int Abox, Bbox;

	for(Abox = 0; Abox < ZONES; Abox++ )
	{
		leftA = a[Abox].x;
		rightA = a[Abox].x + a[Abox].w;
		topA = a[Abox].y;
		bottomA = a[Abox].y + a[Abox].h;

		for(Bbox = 0; Bbox < ZONES; Bbox++ )
		{
			leftB = b[Bbox].x;
			rightB = b[Bbox].x + b[Bbox].w;
			topB = b[Bbox].y;
			bottomB = b[Bbox].y + b[Bbox].h;

			if(
					((bottomA <= topB)
					|| (topA >= bottomB)
					|| (rightA <= leftB)
					|| (leftA >= rightB))
					== 0)
				return 1;
		}
	}

	return 0;
}

short loadMedia(void)
{
	if(LTexture_loadFromFile(&gDotTexture, "dot.bmp"))
		return -1;
	return 0;
}

void close_all(void)
{
	free_texture(&gDotTexture);

	SDL_GameControllerClose(gGameController);
	gGameController = NULL;

	SDL_DestroyRenderer(gRenderer);
	SDL_DestroyWindow(gWindow);
	gWindow = NULL;
	gRenderer = NULL;

	IMG_Quit();
	SDL_Quit();
}

/*
 * Before we go into the main loop we declare our dot and the other dot we'll
 * be colliding against.
 *
 * Once again in the main loop we handle events for the dot, move with
 * collision check for the dot, and then finally we render our objects.
 */
int main(int argc, char* argv[])
{
	if(init())
		goto equit;

	if(loadMedia())
		goto equit;

	SDL_Event e;

	Dot dot;
	Dot_init(&dot, 0, 0);
	
	Dot otherDot;
	Dot_init(&otherDot, SCREEN_WIDTH / 4, SCREEN_HEIGHT / 4);
	
	while(1)
	{
		while(SDL_PollEvent(&e) != 0)
		{
			switch (e.type) {
			case SDL_QUIT:
				goto equit;
			case SDL_CONTROLLERAXISMOTION:
				handle_axis_motion(&e, &dot);
				break;
			case SDL_CONTROLLERDEVICEADDED:
				if (!gGameController) {
					gGameController = SDL_GameControllerOpen(e.cdevice.which);
				}
				break;
			case SDL_CONTROLLERDEVICEREMOVED:
				if (gGameController && e.cdevice.which == SDL_JoystickInstanceID(
						SDL_GameControllerGetJoystick(gGameController))) {
					SDL_GameControllerClose(gGameController);
					gGameController = NULL;
				}
				break;
			default:
				handle_keyboard_events(&dot, &e);
			}
		}

		Dot_move(&dot, otherDot.mColliders);

		SDL_SetRenderDrawColor(gRenderer, 0xFF, 0xFF, 0xFF, 0xFF);
		SDL_RenderClear(gRenderer);
		
		Dot_render(&dot);
		Dot_render(&otherDot);

		SDL_RenderPresent(gRenderer);
		SDL_Delay(16);
	}
equit:
	close_all();

	return 0;
}

/*
 * A questions I get asked a lot is how to make a function that loads an image
 * and auto generates the set of collision boxes for per pixel collision
 * detection. The answer is simple:
 * 
 * Don't.
 * 
 * In most games, you don't want 100% accuracy. The more collision boxes you
 * have the more collision checks you have and the slower it is. What most
 * games go for is close enough, like in Street Fighter:
 * 
 * The results are not pixel perfect but they are close enough.
 * 
 * Also there's one optimization we could have done here. We could have had a
 * bounding box for the dot that encapsulates all the other collision boxes and
 * then checks that one first before getting to the per-pixel collison boxes.
 * This does add one more collision detection, but since it is much more likely
 * that two objects do not collide it will more likely save us additional
 * collision detection. In games, this is usually done with a tree structure
 * that has different levels of detail to allow for early outs to prevent
 * unneeded checks at the per-pixel level. Like in previous tutorials, tree
 * structures are outside the scope of these tutorials.
 */
