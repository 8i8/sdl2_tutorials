OBJS = 36_multiple_windows.c

CC = gcc

# -fsanitize=address -fno-omit-frame-pointer
COMPILER_FLAGS = -g -Wall -Wextra -pedantic

LINKER_FLAGS = -lSDL2

OBJ_NAME = p36_multiple_windows

all: $(OBJS)
	$(CC) $(OBJS) $(COMPILER_FLAGS) $(LINKER_FLAGS) -o $(OBJ_NAME)
