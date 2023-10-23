APP = p03_event_driven_programming
OBJ = 03_event_driven_programming.c
INC = /opt/homebrew/Cellar/sdl2/2.28.4

CC = clang -arch arm64

#CPPFLAGS specifies the additional compilation options we're using
CPPFLAGS += $(foreach D,$(INC),-I$(D)/include)

# CFLAGS += -g -Wall -Werror -pedantic
CFLAGS += -target arm64-apple-darwin 
#CFLAGS += -fsanitize=address -fno-omit-frame-pointer 
#CFLAGS += -v

#LDFLAGS specifies the libraries we're linking against
LDFLAGS += $(foreach D,$(INC),-L$(D)/lib)
LDFLAGS += -lSDL2

#This is the target that compiles our executable
all : $(OBJ)
	$(CC) $(OBJ) $(CPPFLAGS) $(CFLAGS) $(LDFLAGS) -o $(APP)
