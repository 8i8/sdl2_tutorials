OBJS = 49_mutexes_and_conditions.c

CC = gcc

COMPILER_FLAGS = -g -Wall -Wextra -pedantic -fsanitize=address -fno-omit-frame-pointer -fpermissive

LINKER_FLAGS = -lSDL2 -lSDL2_image

OBJ_NAME = p49_mutexes_and_conditions

all: $(OBJS)
	$(CC) $(OBJS) $(COMPILER_FLAGS) $(LINKER_FLAGS) -o $(OBJ_NAME)
