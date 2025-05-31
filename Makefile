# IMPORTANT: this Makefile has not been tested on Linux yet!

CC = gcc
CFLAGS = -g -Wall -Wextra -Wsign-conversion -Wpedantic -std=c11 -Wno-unused-parameter -Werror=incompatible-pointer-types
# CFLAGS = -O2 -Wall -Wextra -Wsign-conversion -Wpedantic -std=c11 -Wno-unused-parameter -Werror=incompatible-pointer-types

ifeq ($(OS), Windows_NT)
	CFLAGS += -Ilib/SDL2_win/include -Ilib/lua-5.4.7/src
	# LDFLAGS = lib/lua-5.4.7/src/liblua.a -Llib/SDL2_win/lib -lmingw32 -lSDL2main -lSDL2 -lSDL2_mixer -lm -mwindows
	LDFLAGS = lib/lua-5.4.7/src/liblua.a -Llib/SDL2_win/lib -lmingw32 -lSDL2main -lSDL2 -lSDL2_mixer -lm
	EXECUTABLE = zinc95.exe
else
	EXECUTABLE = zinc95
	
	UNAME_S := $(shell uname -s)
	ifeq ($(UNAME_S), Linux)
		CFLAGS += -Ilib/lua-5.4.7/src
		LDFLAGS = lib/lua-5.4.7/src/liblua.a -lSDL2 -lSDL2_mixer -lm
	endif
	ifeq ($(UNAME_S), Darwin)
		CFLAGS += -Ilib/SDL2_mac/include -Ilib/lua-5.4.7/src
		LDFLAGS = lib/lua-5.4.7/src/liblua.a -Llib/SDL2_mac/lib -lSDL2 -lSDL2_mixer -lm
	endif
endif

SRC = src/main.c src/computer.c src/res.c \
src/backend/window.c src/backend/sdl2.c src/backend/input.c src/backend/math2d.c src/backend/gfx.c src/backend/gui.c src/backend/text_file.c src/backend/audio.c src/backend/mem.c src/backend/stb_image.c\
src/editor/menu.c  src/editor/code.c src/editor/sprite.c src/editor/map.c \
src/editor/sound.c src/editor/shared.c \
src/api/api.c src/api/lua_api.c

OBJ = $(SRC:.c=.o)

all: libs app

libs:
	cd lib/lua-5.4.7 && \
	if [ "$(OS)" = "Windows_NT" ]; then \
		make all PLAT=mingw; \
	elif [ "$(shell uname -s)" = "Linux" ]; then \
		make all PLAT=linux; \
	elif [ "$(shell uname -s)" = "Darwin" ]; then \
		make all PLAT=macosx; \
	fi

app: $(OBJ)
	$(CC) -o $(EXECUTABLE) $^ $(LDFLAGS)

%.o: %.c
	$(CC) $(CFLAGS) -o $@ -c $<

clean:
	find . -type f -name "*.o" -delete && rm -f $(EXECUTABLE) && rm lib/lua-5.4.7/src/liblua.a

clean_app:
	find . -type f -name "*.o" -delete
