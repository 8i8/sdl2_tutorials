APP = p33_file_reading_and_writing
OBJ = 33_file_reading_and_writing.c
INC = /opt/homebrew/Cellar/sdl2/2.28.4 \
	/opt/homebrew/Cellar/sdl2_image/2.6.3_2 \
	/opt/homebrew/Cellar/sdl2_ttf/2.20.2
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
LDFLAGS += -lSDL2 -lSDL2_image -lSDL2_ttf

# Compilation target
all : $(OBJ)
	$(CC) $(OBJ) $(CPPFLAGS) $(CFLAGS) $(LDFLAGS) -o $(APP)
