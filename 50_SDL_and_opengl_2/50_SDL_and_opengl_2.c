/*
 * This program demonstrates the use of openGL 2 in SDL.
 */
#include <SDL2/SDL.h>
#include <SDL2/SDL_opengl.h>
#include <GL/gl.h>
#include <GL/glu.h>
#include <stdio.h>

#define SCREEN_WIDTH	640
#define SCREEN_HEIGHT	480

SDL_Window* gWindow = NULL;
SDL_GLContext gContext;
short gRenderQuad = 1;

short initGL()
{
	GLenum error = GL_NO_ERROR;

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	
	error = glGetError();
	if(error != GL_NO_ERROR) {
		SDL_Log("%s(), GL_PROJECTION failed. %s\n",
				__func__, gluErrorString(error));
		return -1;
	}

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	error = glGetError();
	if(error != GL_NO_ERROR) {
		SDL_Log("%s(), GL_MODELVIEW failed. %s\n",
				__func__, gluErrorString(error));
		return -1;
	}
	
	glClearColor(0.f, 0.f, 0.f, 1.f);
	
	error = glGetError();
	if(error != GL_NO_ERROR) {
		SDL_Log("%s(), glClearColor failed. %s\n",
				__func__, gluErrorString(error));
		return -1;
	}
	
	return 0;
}

short init()
{
	if(SDL_Init(SDL_INIT_VIDEO) < 0) {
		SDL_Log("%s(), SDL_Init failed. %s", __func__, SDL_GetError());
		return -1;
	}

	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 2);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 1);

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

	gContext = SDL_GL_CreateContext(gWindow);
	if(gContext == NULL) {
		printf("OpenGL context could not be created! SDL Error: %s\n",
				SDL_GetError());
		return -1;
	}

	if(SDL_GL_SetSwapInterval(1) < 0)
		printf("Warning: Unable to set VSync! SDL Error: %s\n",
				SDL_GetError());

	if(initGL()) {
		printf("Unable to initialize OpenGL!\n");
		return -1;
	}

	return 0;
}

void handleKeys(unsigned char key)
{
	if(key == 'q')
		gRenderQuad = !gRenderQuad;
}

void update()
{
}

void render()
{
	glClear(GL_COLOR_BUFFER_BIT);
	
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
	SDL_DestroyWindow(gWindow);
	gWindow = NULL;

	SDL_Quit();
}

int main(int argc, char* args[])
{
	if(init())
		goto equit;

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
				handleKeys(e.text.text[0]);
			}
		}

		render();
		
		SDL_GL_SwapWindow(gWindow);
	}
	
	SDL_StopTextInput();
equit:
	close_all();

	return 0;
}
