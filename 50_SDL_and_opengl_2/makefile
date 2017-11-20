OBJS = 50_SDL_and_opengl_2.c

CC = gcc

COMPILER_FLAGS = -g -Wall -Wextra -pedantic -fsanitize=address -fno-omit-frame-pointer -lGL -lGLU 

LINKER_FLAGS = -lSDL2 -lSDL2_image -lSDL2_ttf

OBJ_NAME = p50_SDL_and_opengl_2

all: $(OBJS)
	$(CC) $(OBJS) $(COMPILER_FLAGS) $(LINKER_FLAGS) -o $(OBJ_NAME)
