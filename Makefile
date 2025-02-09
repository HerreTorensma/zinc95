CC = gcc
CFLAGS = -g -Wall -Wextra -Wsign-conversion -Wpedantic -Wconversion -std=c99

ifeq ($(OS), Windows_NT)
	LDFLAGS = -Ilib/SDL2_win/include -Llib/SDL2_win/lib -lmingw32 -lSDL2main -lSDL2 -lSDL2_mixer -lm
	EXECUTABLE = zinc95.exe
else
	EXECUTABLE = zinc95
	
	UNAME_S := $(shell uname -s)
	ifeq ($(UNAME_S), Linux)
		LDFLAGS = -lSDL2 -lSDL2_mixer -lm
	endif
	ifeq ($(UNAME_S), Darwin)
		LDFLAGS = -Ilib/SDL2_mac/include -Llib/SDL2_mac/lib -lSDL2 -lSDL2_mixer -lm
	endif
endif

SRC = src/main.c src/backend/backend.c src/backend/sdl2.c src/computer.c src/util/util.c src/api/api.c
OBJ = $(SRC:.c = .o)

all: $(OBJ)
	$(CC) -o $(EXECUTABLE) $^ $(LDFLAGS)

%.o: %.c
	$(CC) $(CFLAGS) -o $@ -c $<
