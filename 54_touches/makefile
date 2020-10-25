OBJS = 54_touches.cpp

CC = g++

COMPILER_FLAGS = -g -Wall -Wextra -pedantic -fsanitize=address -fno-omit-frame-pointer -fpermissive

LINKER_FLAGS = -lSDL2 -lSDL2_image

OBJ_NAME = p54_touches

all: $(OBJS)
	$(CC) $(OBJS) $(COMPILER_FLAGS) $(LINKER_FLAGS) -o $(OBJ_NAME)
