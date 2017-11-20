/*This source code copyrighted by Lazy Foo' Productions (2004-2015)
and may not be redistributed without written permission.*/

//Using SDL, SDL OpenGL, standard IO, and, strings
#include <SDL2/SDL.h>
#include <SDL2/SDL_opengl.h>
#include <GL/gl.h>
#include <GL/glu.h>
#include <stdio.h>
#include <string>

//Screen dimension constants
const int SCREEN_WIDTH = 640;
const int SCREEN_HEIGHT = 480;

bool init();		//Starts up SDL, creates window, and initializes OpenGL
bool initGL();		//Initializes matrices and clear color
void update();		//Per frame update
void render();		//Renders quad to the screen
void close();		//Frees media and shuts down SDL
void handleKeys(unsigned char key, int x, int y);//Input handler

SDL_Window* gWindow = NULL;	//The window we'll be rendering to
SDL_GLContext gContext;		//OpenGL context
bool gRenderQuad = true;	//Render flag	

bool init()
{
	//Initialize SDL
	if(SDL_Init(SDL_INIT_VIDEO) < 0) {
		printf("SDL could not initialize! SDL Error: %s\n",
				SDL_GetError());
		return -1;
	}

	//Use OpenGL 2.1
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 2);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 1);

	//Create window
	gWindow = SDL_CreateWindow(
					"SDL Tutorial",
					SDL_WINDOWPOS_UNDEFINED,
					SDL_WINDOWPOS_UNDEFINED,
					SCREEN_WIDTH,
					SCREEN_HEIGHT,
					SDL_WINDOW_OPENGL
					| SDL_WINDOW_SHOWN);
	if(gWindow == NULL) {
		printf("Window could not be created! SDL Error: %s\n",
				SDL_GetError());
		return -1;
	}

	//Create context
	gContext = SDL_GL_CreateContext(gWindow);
	if(gContext == NULL) {
		printf("OpenGL context could not be created! SDL Error: %s\n",
				SDL_GetError());
		return -1;
	}

	//Use Vsync
	if(SDL_GL_SetSwapInterval(1) < 0)
		printf("Warning: Unable to set VSync! SDL Error: %s\n",
				SDL_GetError());

	//Initialize OpenGL
	if(initGL()) {
		printf("Unable to initialize OpenGL!\n");
		return -1;
	}

	return 0;
}

bool initGL()
{
	GLenum error = GL_NO_ERROR;

	//Initialize Projection Matrix
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	
	//Check for error
	error = glGetError();
	if(error != GL_NO_ERROR) {
		printf("Error initializing OpenGL! %s\n", gluErrorString(error));
		return -1;
	}

	//Initialize Modelview Matrix
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	//Check for error
	error = glGetError();
	if(error != GL_NO_ERROR) {
		printf("Error initializing OpenGL! %s\n", gluErrorString(error));
		return -1;
	}
	
	//Initialize clear color
	glClearColor(0.f, 0.f, 0.f, 1.f);
	
	//Check for error
	error = glGetError();
	if(error != GL_NO_ERROR) {
		printf("Error initializing OpenGL! %s\n", gluErrorString(error));
		return -1;
	}
	
	return 0;
}

void handleKeys(unsigned char key, int x, int y)
{
	//Toggle quad
	if(key == 'q')
		gRenderQuad = !gRenderQuad;
}

void update()
{
	//No per frame update needed
}

void render()
{
	//Clear color buffer
	glClear(GL_COLOR_BUFFER_BIT);
	
	//Render quad
	if(gRenderQuad) {
		glBegin(GL_QUADS);
		glVertex2f(-0.5f, -0.5f);
		glVertex2f(0.5f, -0.5f);
		glVertex2f(0.5f, 0.5f);
		glVertex2f(-0.5f, 0.5f);
		glEnd();
	}
}

void close_all()
{
	//Destroy window	
	SDL_DestroyWindow(gWindow);
	gWindow = NULL;

	//Quit SDL subsystems
	SDL_Quit();
}

int main(int argc, char* args[])
{
	if(init()) {
		printf("Failed to initialize!\n");
		goto equit;
	}

	SDL_Event e;
	SDL_StartTextInput();

	int x, y;

	while(1)
	{
		while(SDL_PollEvent(&e) != 0) {
			if(e.type == SDL_QUIT)
				goto equit;

			else if(e.type == SDL_TEXTINPUT) {
				x = 0, y = 0;
				SDL_GetMouseState(&x, &y);
				handleKeys(e.text.text[0], x, y);
			}
		}

		//Render quad
		render();
		
		//Update screen
		SDL_GL_SwapWindow(gWindow);
	}
	
	//Disable text input
	SDL_StopTextInput();
equit:
	//Free resources and close SDL
	close_all();

	return 0;
}
