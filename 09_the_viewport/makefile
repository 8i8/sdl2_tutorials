OBJS = 09_the_viewport.c

CC = gcc

COMPILER_FLAGS = -g -Wall -Wextra -pedantic -fsanitize=address -fno-omit-frame-pointer

LINKER_FLAGS = -lSDL2 -lSDL2_image

OBJ_NAME = p09_the_viewport

all: $(OBJS)
	$(CC) $(OBJS) $(COMPILER_FLAGS) $(LINKER_FLAGS) -o $(OBJ_NAME)
