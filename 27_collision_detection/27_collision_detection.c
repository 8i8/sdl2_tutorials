/*
 * Collision Detection
 *
 * In games you often need to tell if two objects hit each other. For simple
 * games, this is usually done with bounding box collision detection.
 */
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>

#define SCREEN_WIDTH	640
#define SCREEN_HEIGHT	480
#define DOT_WIDTH	20
#define DOT_HEIGHT	20
#define DOT_VEL		5
#define DOT_JOY_VEL	1
#define JOYSTICK_DEAD_ZONE	8000

typedef struct {
	SDL_Texture *mTexture;
	int mWidth;
	int mHeight;
} LTexture;

/*
 * Here is the dot from the motion tutorial with some new features. The move
 * function takes in a rectangle that is the collision box for the wall and the
 * dot has a data member mCollider to represent the collision box.
 * We're also declaring a function to check collision between two boxes.
 */
typedef struct {
	int mPosX, mPosY;
	int mVelX, mVelY;
	SDL_Rect mCollider;
} Dot;

short checkCollision(SDL_Rect *a, SDL_Rect *b);

SDL_Window* gWindow = NULL;
SDL_Renderer* gRenderer = NULL;
SDL_GameController* gGameController = NULL;
LTexture gDotTexture;

short init(void)
{
	if(SDL_Init(SDL_INIT_VIDEO | SDL_INIT_JOYSTICK) < 0) {
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
 * In the contructor we should make sure the collider's dimensions are set. 
 */
Dot *Dot_init(Dot *d)
{
	d->mPosX = 0;
	d->mPosY = 0;
	d->mVelX = 0;
	d->mVelY = 0;
	d->mCollider.w = DOT_WIDTH;
	d->mCollider.h = DOT_HEIGHT;
	return d;
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
			case SDLK_DOWN:	d->mVelY -= DOT_VEL; break;
			case SDLK_LEFT: d->mVelX += DOT_VEL; break;
			case SDLK_RIGHT:d->mVelX -= DOT_VEL; break;
		}
	}
}

void handle_axis_motion(SDL_Event *e, Dot *d) {
	if(e->jaxis.which == 0) {
		if(e->jaxis.axis == 0)
		{
			if(e->jaxis.value < -JOYSTICK_DEAD_ZONE)
				d->mVelX -= DOT_JOY_VEL;
			else if(e->jaxis.value > JOYSTICK_DEAD_ZONE)
				d->mVelX += DOT_JOY_VEL;
			else
				d->mVelX = 0;
		}
		else if(e->jaxis.axis == 1)
		{
			if(e->jaxis.value < -JOYSTICK_DEAD_ZONE)
				d->mVelY -= DOT_JOY_VEL;
			else if(e->jaxis.value > JOYSTICK_DEAD_ZONE)
				d->mVelY += DOT_JOY_VEL;
			else
				d->mVelY = 0;
		}
	}
}

/*
 * Here is the new moving function that now checks if we hit the wall. It works
 * much like before only now it makes the dot move back if we go off the screen
 * or hit the wall.
 *
 * First we move the dot along the x axis, but we also have to change the
 * position of the collider. Whenever we change the dot's position, the
 * collider's position has to follow. Then we check of the dot has gone off
 * screen or hit the wall. If it does we move the dot back along the x axis.
 * Finally, we do this again for motion on the y axis.
 */
void Dot_move(Dot *d, SDL_Rect *wall)
{
	d->mPosX += d->mVelX;
	d->mCollider.x = d->mPosX;

	if(
			(d->mPosX < 0)
			|| (d->mPosX + DOT_WIDTH > SCREEN_WIDTH)
			|| checkCollision(&d->mCollider, wall))
	{
		d->mPosX -= d->mVelX;
		d->mCollider.x = d->mPosX;
	}

	d->mPosY += d->mVelY;
	d->mCollider.y = d->mPosY;

	if(
			(d->mPosY < 0)
			|| (d->mPosY + DOT_HEIGHT > SCREEN_HEIGHT)
			|| checkCollision(&d->mCollider, wall))
	{
		d->mPosY -= d->mVelY;
		d->mCollider.y = d->mPosY;
	}
}

/*
 * Here is where the collision detection happens. This code calculates the
 * top/bottom and left/right of each of the collison boxes.
 *
 * Here is where we do our separating axis test. First we check the top and
 * bottom of the boxes to see if they are separated along the y axis. Then we
 * check the left/right to see if they are separated on the x axis. If there is
 * any separation, then there is no collision and we return false. If we cannot
 * find any separation, then there is a collision and we return true.
 *
 * Note: SDL does have some built in collision detection functions, but for
 * this tutorial set we'll be hand rolling our own. Mainly because it's
 * important to know how these work and secondly because if you can roll your
 * own you can use your collision detection with SDL rendering, OpenGL,
 * Direct3D, Mantle, Metal, or any other rendering API.
 */
short checkCollision(SDL_Rect *dot, SDL_Rect *wall)
{
	int dot_left, wall_left;
	int dot_right, wall_right;
	int dot_top, wall_top;
	int dot_bottom, wall_bottom;

	dot_left	= dot->x;
	dot_right	= dot->x + dot->w;
	dot_top		= dot->y;
	dot_bottom	= dot->y + dot->h;

	wall_left	= wall->x;
	wall_right	= wall->x + wall->w;
	wall_top	= wall->y;
	wall_bottom	= wall->y + wall->h;

	if(dot_bottom <= wall_top)
		return 0;	
	if(dot_top >= wall_bottom)
		return 0;
	if(dot_right <= wall_left)
		return 0;
	if(dot_left >= wall_right)
		return 0;

	return 1;
}

void Dot_render(Dot *d)
{
	LTexture_render(&gDotTexture, d->mPosX, d->mPosY, NULL);
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
 * Before we enter the main loop, we declare the dot and define the postion and
 * dimensions of the wall.
 *
 * Here is our main loop with the dot handling events, moving while checking
 * for collision against the wall and finally rendering the wall and the dot
 * onto the screen.
 */
int main(int argc, char* argv[])
{
	if(init())
		goto equit;

	if(loadMedia())
		goto equit;

	SDL_Event e;
	Dot dot;
	Dot_init(&dot);

	SDL_Rect wall;
	wall.x = 300;
	wall.y = 40;
	wall.w = 40;
	wall.h = 400;
	
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

		Dot_move(&dot, &wall);

		SDL_SetRenderDrawColor(gRenderer, 0xFF, 0xFF, 0xFF, 0xFF);
		SDL_RenderClear(gRenderer);

		SDL_SetRenderDrawColor(gRenderer, 0x00, 0x00, 0x00, 0xFF);
		SDL_RenderDrawRect(gRenderer, &wall);
		
		Dot_render(&dot);

		SDL_RenderPresent(gRenderer);
		SDL_Delay(16);
	}
equit:
	close_all();

	return 0;
}
/*
 * These next two sections are for future reference. Odds are if you're reading
 * this tutorial, you're a beginner and this stuff is way too advanced. This is
 * more for down the road when you need to use more advanced collision
 * detection.
 * 
 * Now when you're starting out and just want to make something simple like
 * tetris, this sort of collision detection is fine. For something like a
 * physics simulator things get a much more complicated.
 * 
 * For something like a rigid body simulator, we have our logic do this every
 * frame:
 *
 * 1) Apply all the forces to all the objects in the scene (gravity, wind,
 * propulsion, etc).
 * 
 * 2) Move the objects by applying the acceleration and velocity to the
 * position.
 * 
 * 3) Check collisions for all of the objects and create a set of contacts. A
 * contact is a data structure that typically contains pointers to the two
 * objects that are colliding, a normal vector from the first to the second
 * object, and the amount the objects are penetrating.
 * 
 * 4) Take the set of generated contacts and resolve the collisions. This
 * typically involves checking for contacts again (within a limit) and
 * resolving them.
 * 
 * Now if you're barely learning collision detection, this is out of your
 * league for now. This would take an entire tutorial set (that I currently do
 * not have time to make) to explain it. Not only that it involves vector math
 * and physics which is beyond the scope of these tutorials. Just keep this in
 * mind later on when you need games that have large amounts of colliding
 * objects and are wondering how the over all structure for a physics engine
 * works.
 *
 * Another thing is that the boxes we have here are AABBs or axis aligned
 * bounding boxes. This means they have sides that are aligned with the x and y
 * axis. If you want to have boxes that are rotated, you can still use the
 * separating axis test on OBBs (oriented bounding boxes). Instead of
 * projecting the corners on the x and y axis, you project all of the corners
 * of the boxes on the I and J axis for each of the boxes. You then check if
 * the boxes are separated along each axis. You can extend this further for any
 * type of polygon by projecting all of the corners of each axis along each of
 * the polygon's axis to see if there is any separation. This all involves
 * vector math and this as mentioned before is beyond the scope of this
 * tutorial set. 
 */
