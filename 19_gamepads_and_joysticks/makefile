OBJS = 19_gamepads_and_joysticks.c

CC = gcc

COMPILER_FLAGS = -g -Wall -Wextra -pedantic -fsanitize=address -fno-omit-frame-pointer

LINKER_FLAGS = -lSDL2 -lSDL2_image -lm

OBJ_NAME = 19_gamepads_and_joysticks

all: $(OBJS)
	$(CC) $(OBJS) $(COMPILER_FLAGS) $(LINKER_FLAGS) -o $(OBJ_NAME)
