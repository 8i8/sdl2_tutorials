OBJS = 43_render_to_texture.c

CC = gcc

COMPILER_FLAGS = -g -Wall -Wextra -pedantic -fsanitize=address -fno-omit-frame-pointer

LINKER_FLAGS = -lSDL2 -lSDL2_image

OBJ_NAME = p43_render_to_texture

all: $(OBJS)
	$(CC) $(OBJS) $(COMPILER_FLAGS) $(LINKER_FLAGS) -o $(OBJ_NAME)
