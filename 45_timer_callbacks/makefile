APP = p45_timer_callbacks
OBJ = 45_timer_callbacks.c
INC = /opt/homebrew/Cellar/sdl2/2.28.4 \
	/opt/homebrew/Cellar/sdl2_image/2.6.3_2
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
LDFLAGS += -lSDL2 -lSDL2_image

# Compilation target
all : $(OBJ)
	$(CC) $(OBJ) $(CPPFLAGS) $(CFLAGS) $(LDFLAGS) -o $(APP)
