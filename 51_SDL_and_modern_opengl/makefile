OBJS = 51_SDL_and_modern_opengl.c

CC = gcc

# -fsanitize=address -fno-omit-frame-pointer
COMPILER_FLAGS = -g -Wall -Wextra -pedantic

LINKER_FLAGS = -lSDL2 -lSDL2_image -lSDL2_ttf -lGLEW -lGLU -lGL

OBJ_NAME = p51_SDL_and_modern_opengl

all: $(OBJS)
	$(CC) $(OBJS) $(COMPILER_FLAGS) $(LINKER_FLAGS) -o $(OBJ_NAME)
