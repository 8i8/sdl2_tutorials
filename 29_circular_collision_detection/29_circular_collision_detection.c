/*
 * Circular Collision Detection
 *
 * Along with collision boxes, circles are the most common form of collider.
 * Here we'll be checking collision between two circles and a circle and a box.
 *
 * Checking collision between two circles is easy. All you have to do is check
 * whether the distance between the center of each circle is less than the sum
 * of their radii (radii is the plural for radius).
 *
 * For box/circle collision, you have to find the point on the collision box
 * that is closest to the center of the circle. If that point is less than the
 * radius of the circle, there is a collision.
 *
 * For this tutorial we have our collision detection functions for
 * circle/circle and circle/rectangle collisions. We also have a function that
 * calculates the distance between two points squared.
 *
 * Using the distance squared instead of the distance is an optimization we'll
 * go into more detail later.
 *
 * TODO work over the math in this example to understand exactly what is
 * happening and improve the collision detection, as with the pervious example
 * the detection fails when the velocity is greater than one.
 */
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>

#define SCREEN_WIDTH	640
#define SCREEN_HEIGHT	480
#define DOT_WIDTH		20
#define DOT_HEIGHT		20
#define DOT_VEL			5
#define DOT_JOY_VEL		1
#define JOYSTICK_DEAD_ZONE	10000

/*
 * SDL has a built in rectangle structure, but we have to make our own circle
 * structure with a position and radius.
 */
typedef struct {
	int x, y;
	int r;
} Circle;

typedef struct {
	SDL_Texture *mTexture;
	int mWidth;
	int mHeight;
} LTexture;

/*
 * Here is the dot struct from previous collision detection tutorials with some
 * more additons. The related move function takes in a circle and a rectangle
 * to check collision against when moving. We also now have a circle collider
 * instead of a rectangle collider.
 */
typedef struct {
	int mPosX, mPosY;
	int mVelX, mVelY;
	Circle mCollider;
} Dot;

short check_collision_circ(Circle *a, Circle *b);
short check_collision_rect(Circle *a, SDL_Rect *b);
double distanceSquared(int x1, int y1, int x2, int y2);
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
					SDL_RENDERER_ACCELERATED
					| SDL_RENDERER_PRESENTVSYNC);
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
 * The init function takes in a position and initializes the colliders and
 * velocity.
 */
void Dot_init(Dot *d, int x, int y)
{
	d->mPosX = x;
	d->mPosY = y;

	d->mCollider.r = DOT_WIDTH / 2;

	d->mVelX = 0;
	d->mVelY = 0;

	Dot_shiftColliders(d);
}

/*
 * Like in previous collision detection tutorials, we move along the x axis,
 * check collision against the edges of the screen, and check against the other
 * scene objects. If the dot hit something we move back. As always, whenever
 * the dot moves its colliders move with it.
 *
 * Then we do this again for the y axis.
 */
void Dot_move(Dot *d, SDL_Rect *square, Circle *circle)
{
	d->mPosX += d->mVelX;
	Dot_shiftColliders(d);

	if(
			(d->mPosX - d->mCollider.r < 0)
			|| (d->mPosX + d->mCollider.r > SCREEN_WIDTH)
			|| check_collision_rect(&d->mCollider, square)
			|| check_collision_circ(&d->mCollider, circle))
	{
		d->mPosX -= d->mVelX;
		Dot_shiftColliders(d);
	}

	d->mPosY += d->mVelY;
	Dot_shiftColliders(d);

	if(
			(d->mPosY - d->mCollider.r < 0)
			|| (d->mPosY + d->mCollider.r > SCREEN_HEIGHT)
			|| check_collision_rect(&d->mCollider, square)
			|| check_collision_circ(&d->mCollider, circle))
	{
		d->mPosY -= d->mVelY;
		Dot_shiftColliders(d);
	}
}

/*
 * The rendering code is a little different. SDL_Rects have their position at
 * the top left where our circle structure has the position at the center. This
 * means we need to offset the render position to the top left of the circle by
 * subtracting the radius from the x and y position.
 */
void Dot_render(Dot *d)
{
	LTexture_render(
			&gDotTexture,
			d->mPosX - d->mCollider.r,
			d->mPosY - d->mCollider.r,
			NULL);
}

/*
 * Here is our circle to circle collision detector. It simply checks if the
 * distance squared between the centers is less than the sum of the radii
 * squared. If it is, there is a collison.
 *
 * Why are we using the distance squared as opposed to the plain distance?
 * Because to calculate the distance involves a square root and calculating a
 * square root is a relatively expensive operation. Fortunately if x > y then
 * x^2 > y^2, so we can save a square root operation by just comparing the
 * distance squared.
 */
short check_collision_circ(Circle *a, Circle *b)
{
	int totalRadiusSquared = a->r + b->r;
	totalRadiusSquared = totalRadiusSquared * totalRadiusSquared;

	if(distanceSquared(a->x, a->y, b->x, b->y) < (totalRadiusSquared))
		return -1;

	return 0;
}

void Dot_shiftColliders(Dot *d)
{
	d->mCollider.x = d->mPosX;
	d->mCollider.y = d->mPosY;
}

/*
 * To check if a box and circle collided we need to find the closest point on
 * the box.
 *
 * If the circle's center is to the left of the box, the x position of the
 * closest point is on the left side of the box.
 *
 * If the circle's center is to the right of the box, the x position of the
 * closest point is on the right side of the box.
 *
 * If the circle's center is inside of the box, the x position of the closest
 * point is the same as the x position of the circle.
 *
 * Here we find the closest y position much like we did the x position. If the
 * distance squared between the closest point on the box and the center of the
 * circle is less than the circle's radius squared, then there is a collision.
 */
short check_collision_rect(Circle *a, SDL_Rect *b)
{
	int cX, cY;

	if(a->x < b->x)
		cX = b->x;
	else if(a->x > b->x + b->w)
		cX = b->x + b->w;
	else
		cX = a->x;

	if(a->y < b->y)
		cY = b->y;
	else if(a->y > b->y + b->h)
		cY = b->y + b->h;
	else
		cY = a->y;

	if(distanceSquared(a->x, a->y, cX, cY) < a->r * a->r)
		return -1;

	return 0;
}

/*
 * Here is the distance squared function. It's just a distance calculation (
 * squareRoot( x^2 + y^2 ) ) without the square root.
 */
double distanceSquared(int x1, int y1, int x2, int y2)
{
	int deltaX = x2 - x1;
	int deltaY = y2 - y1;
	return deltaX*deltaX + deltaY*deltaY;
}

short loadMedia(void)
{
	if(LTexture_loadFromFile(&gDotTexture, "dot.bmp") < 0)
		return -1;
	return 0;
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
 * Before we enter the main loop we define the scene objects.
 *
 * Finally in our main loop we handle input, move the dot with collision check
 * and render the scene objects to the screen.
 */
int main(int argc, char* argv[])
{
	if(init())
		goto equit;

	if(loadMedia())
		goto equit;

	SDL_Event e;

	Dot dot;
	Dot otherDot;
	Dot_init(&dot, DOT_WIDTH / 2, DOT_HEIGHT / 2);
	Dot_init(&otherDot, SCREEN_WIDTH / 4, SCREEN_HEIGHT / 4);

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

		Dot_move(&dot, &wall, &otherDot.mCollider);

		SDL_SetRenderDrawColor(gRenderer, 0xFF, 0xFF, 0xFF, 0xFF);
		SDL_RenderClear(gRenderer);

		SDL_SetRenderDrawColor(gRenderer, 0x00, 0x00, 0x00, 0xFF);
		SDL_RenderDrawRect(gRenderer, &wall);
		
		Dot_render(&dot);
		Dot_render(&otherDot);

		SDL_RenderPresent(gRenderer);
	}
equit:
	close_all();

	return 0;
}

