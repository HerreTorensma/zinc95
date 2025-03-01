# IMPORTANT: right now this Makefile only compiles Lua on Windows, I will fix this soon

CC = gcc
CFLAGS = -g -Wall -Wextra -Wsign-conversion -Wpedantic -Wconversion -std=c11 -Wno-unused-parameter

ifeq ($(OS), Windows_NT)
	CFLAGS += -Ilib/SDL2_win/include -Ilib/lua-5.4.7/src
	LDFLAGS = lib/lua-5.4.7/src/liblua.a -Llib/SDL2_win/lib -lmingw32 -lSDL2main -lSDL2 -lSDL2_mixer -lm
	EXECUTABLE = zinc95.exe
else
	EXECUTABLE = zinc95
	
	UNAME_S := $(shell uname -s)
	ifeq ($(UNAME_S), Linux)
		LDFLAGS = -lSDL2 -lSDL2_mixer -lm
	endif
	ifeq ($(UNAME_S), Darwin)
		CFLAGS += -Ilib/SDL2_mac/include
		LDFLAGS = -Llib/SDL2_mac/lib -lSDL2 -lSDL2_mixer -lm
	endif
endif

SRC = src/main.c src/backend/backend.c src/backend/sdl2.c src/computer.c src/util/util.c src/api/api.c src/workspaces/sprite.c src/backend/input.c src/res.c src/workspaces/menu.c src/workspaces/console.c src/api/lua_api.c
OBJ = $(SRC:.c=.o)

all: libs app

libs:
	cd lib/lua-5.4.7 && make all PLAT=mingw

app: $(OBJ)
	$(CC) -o $(EXECUTABLE) $^ $(LDFLAGS)

%.o: %.c
	$(CC) $(CFLAGS) -o $@ -c $<

clean:
	find . -type f -name "*.o" -delete && rm $(EXECUTABLE)
