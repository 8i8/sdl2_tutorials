OBJS = 23_advanced_timers.c

CC = gcc

COMPILER_FLAGS = -g -Wall -Wextra -pedantic -fsanitize=address -fno-omit-frame-pointer

LINKER_FLAGS = -lSDL2 -lSDL2_image -lSDL2_ttf

OBJ_NAME = p23_advanced_timers

all: $(OBJS)
	$(CC) $(OBJS) $(COMPILER_FLAGS) $(LINKER_FLAGS) -o $(OBJ_NAME)
