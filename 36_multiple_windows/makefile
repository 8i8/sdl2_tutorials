APP = p36_multiple_windows
OBJ = 36_multiple_windows.c
INC = /opt/homebrew/Cellar/sdl2/2.28.4
CC = clang -arch arm64

# Preprocessor flags
CPPFLAGS += $(foreach D,$(INC),-I$(D)/include)

# Compiler flags
# CFLAGS += -g -Wall -Werror -pedantic
CFLAGS += -target arm64-apple-darwin 
#CFLAGS += -fsanitize=address -fno-omit-frame-pointer 
#CFLAGS += -v

# Linker flags
LDFLAGS += $(foreach D,$(INC),-L$(D)/lib)
LDFLAGS += -lSDL2

# Compilation target
all : $(OBJ)
	$(CC) $(OBJ) $(CPPFLAGS) $(CFLAGS) $(LDFLAGS) -o $(APP)
