/*This source code copyrighted by Lazy Foo' Productions (2004-2015)
and may not be redistributed without written permission.*/

/*
 * TODO the openGL is not rendering.
 */

//Using SDL, SDL OpenGL, GLEW, standard IO, and strings
#include <SDL2/SDL.h>
#include <GL/glew.h>
#include <stdio.h>

//Screen dimension constants
const int SCREEN_WIDTH = 640;
const int SCREEN_HEIGHT = 480;

enum { false, true };

int init(void);		//Starts up SDL, creates window, and initializes OpenGL
int initGL(void);		//Initializes rendering program and clear color
void update(void);		//Per frame update
void render(void);		//Renders quad to the screen
void close(void);		//Frees media and shuts down SDL
void handleKeys(unsigned char key, int x, int y);//Input handler

//Shader loading utility programs
int printProgramLog(GLuint program);
int printShaderLog(GLuint shader);

SDL_Window* gWindow = NULL;	//The window we'll be rendering to
SDL_GLContext gContext;		//OpenGL context
int gRenderQuad = true;	//Render flag

//Graphics program
GLuint gProgramID = 0;
GLint gVertexPos2DLocation = -1;
GLuint gVBO = 0;
GLuint gIBO = 0;

int test(void)
{
	GLenum z;
	if ((z = glGetError())) {
		printf("error is %d\n", z);
		return 1;
	}
	return 0;
}

int initGL(void)
{
	// Program
	gProgramID = glCreateProgram();
	GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);

	//Get vertex source
	const GLchar* vertexShaderSource[] = {
		"#version 140\nin vec2 LVertexPos2D; void main() { gl_Position = vec4(LVertexPos2D.x, LVertexPos2D.y, 0, 1); }"
	};

	//Set vertex source
	glShaderSource(vertexShader, 1, vertexShaderSource, NULL);
	glCompileShader(vertexShader);

	//Check vertex shader for errors
	GLint vShaderCompiled = GL_FALSE;
	glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &vShaderCompiled);
	if(vShaderCompiled != GL_TRUE) {
		printf("Unable to compile vertex shader %d!\n", vertexShader);
		printShaderLog(vertexShader);
		return -1;
	}

	//Attach vertex shader to program
	glAttachShader(gProgramID, vertexShader);
	//Create fragment shader
	GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);

	//Get fragment source
	const GLchar* fragmentShaderSource[] = {
		"#version 140\nout vec4 LFragment; void main() { LFragment = vec4(1.0, 1.0, 1.0, 1.0); }"
	};

	//Set fragment source
	glShaderSource(fragmentShader, 1, fragmentShaderSource, NULL);

	//Compile fragment source
	glCompileShader(fragmentShader);

	//Check fragment shader for errors
	GLint fShaderCompiled = GL_FALSE;
	glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &fShaderCompiled);
	if(fShaderCompiled != GL_TRUE) {
		printf("Unable to compile fragment shader %d!\n", fragmentShader);
		printShaderLog(fragmentShader);
		return -1;
	}

	//Attach fragment shader to program
	glAttachShader(gProgramID, fragmentShader);

	//Link program
	glLinkProgram(gProgramID);

	//Check for errors
	GLint programSuccess = GL_TRUE;
	glGetProgramiv(gProgramID, GL_LINK_STATUS, &programSuccess);
	if(programSuccess != GL_TRUE) {
		printf("Error linking program %d!\n", gProgramID);
		printProgramLog(gProgramID);
		return -1;
	}

	//Get vertex attribute location
	gVertexPos2DLocation = glGetAttribLocation(gProgramID, "LVertexPos2D");
	if(gVertexPos2DLocation == -1) {
		printf("LVertexPos2D is not a valid glsl program variable!\n");
		return -1;
	}

	//Initialize clear color
	glClearColor(0.f, 0.f, 0.f, 1.f);

	//VBO data
	GLfloat vertexData[] = {
		-0.5f, -0.5f,
		 0.5f, -0.5f,
		 0.5f,	0.5f,
		-0.5f,	0.5f
	};

	//IBO data
	//GLuint indexData[] = { 0, 1, 2, 3 };
	GLuint indexData[] = { 0, 1, 2, 2, 3, 0 };

	//Create VBO
	glGenBuffers(1, &gVBO);
	glBindBuffer(GL_ARRAY_BUFFER, gVBO);
	glBufferData(
			GL_ARRAY_BUFFER,
			2 * 4 * sizeof(GLfloat),
			vertexData,
			GL_STATIC_DRAW);

	//Create IBO
	glGenBuffers(1, &gIBO);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, gIBO);
	glBufferData(
			GL_ELEMENT_ARRAY_BUFFER,
			4 * sizeof(GLuint),
			indexData,
			GL_STATIC_DRAW);

	return 0;
}

int init(void)
{
	if(SDL_Init(SDL_INIT_VIDEO) < 0) {
		printf("SDL could not initialize! SDL Error: %s\n",
				SDL_GetError());
		return -1;
	}

	//Use OpenGL 3.1 core
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
	SDL_GL_SetAttribute(SDL_GL_ACCELERATED_VISUAL, 1);

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

	//Initialize GLEW
	glewExperimental = GL_TRUE;
	GLenum glewError = glewInit();
	if(glewError != GLEW_OK) {
		printf("Error initializing GLEW! %s\n",
				glewGetErrorString(glewError));
		return -1;
	}

	//Use Vsync
	if(SDL_GL_SetSwapInterval(1) < 0) {
		printf("Warning: Unable to set VSync! SDL Error: %s\n",
				SDL_GetError());
		return -1;
	}

	//Initialize OpenGL
	if(initGL()) {
		printf("Unable to initialize OpenGL!\n");
		return -1;
	}

	return 0;
}

void handleKeys(unsigned char key, int x, int y)
{
	if(key == 'q')
		gRenderQuad = !gRenderQuad;
}

void update(void)
{
	//No per frame update needed
}

void render(void)
{
	//Render quad
	if(gRenderQuad)
	{
		//Bind program
		glUseProgram(gProgramID);

		//Enable vertex position
		glEnableVertexAttribArray(gVertexPos2DLocation);

		//Set vertex data
		glBindBuffer(GL_ARRAY_BUFFER, gVBO);
		glVertexAttribPointer(
					gVertexPos2DLocation,
					2,
					GL_FLOAT,
					GL_FALSE,
					2 * sizeof(GLfloat),
					NULL);

		//Set index data and render
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, gIBO);
		glDrawElements(GL_TRIANGLE_FAN, 4, GL_UNSIGNED_INT, NULL);
		glDrawElements(GL_TRIANGLE_FAN, 6, GL_UNSIGNED_INT, NULL);
		glDrawElements(GL_TRIANGLE_STRIP, 6, GL_UNSIGNED_INT, NULL);

		//Disable vertex position
		glDisableVertexAttribArray(gVertexPos2DLocation);

		//Unbind program
		glUseProgram(0);
	}
}

void close_all(void)
{
	//Deallocate program
	glDeleteProgram(gProgramID);

	//Destroy window
	SDL_DestroyWindow(gWindow);
	gWindow = NULL;

	//Quit SDL subsystems
	SDL_Quit();
}

int printProgramLog(GLuint program)
{
	//Make sure name is shader
	if(glIsProgram(program) != 0) {
		printf("Name %d is not a program\n", program);
		return -1;
	}

	//Program log length
	int infoLogLength = 0;
	int maxLength = infoLogLength;

	//Get info string length
	glGetProgramiv(program, GL_INFO_LOG_LENGTH, &maxLength);

	//Allocate string
	char* infoLog = (char*)malloc(maxLength);

	//Get info log
	glGetProgramInfoLog(program, maxLength, &infoLogLength, infoLog);
	if(infoLogLength > 0)
		printf("%s\n", infoLog);

	//Deallocate string
	free(infoLog);

	return 0;
}

int printShaderLog(GLuint shader)
{
	//Make sure name is shader
	if(glIsShader(shader) != 0) {
		printf("Name %d is not a shader\n", shader);
		return -1;
	}

	//Shader log length
	int infoLogLength = 0;
	int maxLength = infoLogLength;

	//Get info string length
	glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &maxLength);

	//Allocate string
	char* infoLog = (char*)malloc(maxLength);

	//Get info log
	glGetShaderInfoLog(shader, maxLength, &infoLogLength, infoLog);
	if(infoLogLength > 0)
		printf("%s\n", infoLog);

	//Deallocate string
	free(infoLog);

	return -1;
}

int main(int argc, char* args[])
{
	//Start up SDL and create window
	if(init()) {
		printf("Failed to initialize!\n");
		goto equit;
	}

	printf("Test %s\n", glGetString(GL_VERSION));

	//Event handler
	SDL_Event e;

	//Enable text input
	SDL_StartTextInput();

	int x, y;
	GLuint vao;

	//While application is running
	while(1)
	{
		while(SDL_PollEvent(&e) != 0) {
			if(e.type == SDL_QUIT)
				goto equit;

			//Handle keypress with current mouse position
			else if(e.type == SDL_TEXTINPUT) {
				x = 0, y = 0;
				SDL_GetMouseState(&x, &y);
				handleKeys(e.text.text[0], x, y);
			}
		}

		//Render quad
		glClearColor(0.6, 0.7, 0.2, 1.0);
		glClear(GL_COLOR_BUFFER_BIT);

		glGenVertexArrays(1, &vao);
		glBindVertexArray(vao);

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
