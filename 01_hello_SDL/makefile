#OBJS specifies which files to compile as part of the project
OBJS = 01_hello_SDL.c

#CC specifies which compiler we're using
CC = gcc

#COMPILER_FLAGS specifies the additional compilation options we're using
# -w suppresses all warnings
COMPILER_FLAGS = -Wall -Wextra -pedantic

#LINKER_FLAGS specifies the libraries we're linking against
#-lSDL_image -lSDL_ttf -lSDL_mixer
LINKER_FLAGS = -lSDL2

#OBJ_NAME specifies the name of our exectuable
OBJ_NAME = p01_hello_SDL

#This is the target that compiles our executable
all : $(OBJS)
	$(CC) $(OBJS) $(COMPILER_FLAGS) $(LINKER_FLAGS) -o $(OBJ_NAME)
