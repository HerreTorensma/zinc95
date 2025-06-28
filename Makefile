# TODO: fix on linux

CC = gcc
CFLAGS = -g -Wall -Wextra -Wsign-conversion -Wpedantic -std=c11 -Wno-unused-parameter -Werror=incompatible-pointer-types -funsigned-char

# Source files
SRC = src/main.c src/computer.c src/res.c \
src/backend/window.c src/backend/sdl2.c src/backend/input.c src/backend/math2d.c src/backend/gfx.c src/backend/gui.c src/backend/text_file.c src/backend/audio.c src/backend/mem.c src/backend/stb_image.c src/backend/txt.c \
src/editor/menu.c  src/editor/code.c src/editor/sprite.c src/editor/map.c \
src/editor/sound.c src/editor/shared.c \
src/api/api.c src/api/lua_api.c

OBJ = $(SRC:.c=.o)

# Automatically detect platform
ifeq ($(OS), Windows_NT)
	PLAT = mingw
else
	UNAME_S := $(shell uname -s)
	ifeq ($(UNAME_S), Linux)
		PLAT = linux
	endif
	ifeq ($(UNAME_S), Darwin)
		PLAT = macosx
	endif
endif

ifeq ($(PLAT),mingw)
	CFLAGS += -Iextern/sdl2/include -Iextern/lua-5.4.8/src -Iextern/stb
	LDFLAGS = extern/lua-5.4.8/src/liblua.a -Lextern/sdl2/lib -lmingw32 -lSDL2main -lSDL2 -lm
	EXECUTABLE = zinc95.exe
endif

ifeq ($(PLAT),linux)
	CFLAGS += -Iextern/lua-5.4.8/src -Iextern/stb
	LDFLAGS = extern/lua-5.4.8/src/liblua.a -lSDL2 -lm
	EXECUTABLE = zinc95
endif

ifeq ($(PLAT),macosx)
	CFLAGS += -Iextern/sdl2/include -Iextern/lua-5.4.8/src -Iextern/stb
	LDFLAGS = extern/lua-5.4.8/src/liblua.a -Lextern/sdl2/lib -lSDL2 -lm
	EXECUTABLE = zinc95
endif

all: build

libs: lib_lua lib_sdl2 lib_stb_image

build: $(OBJ)
	$(CC) -o $(EXECUTABLE) $^ $(LDFLAGS)

%.o: %.c
	$(CC) $(CFLAGS) -o $@ -c $<

clean_all:
	find src -type f -name "*.o" -delete
	rm -f $(EXECUTABLE)
	rm -rf extern
	rm -f SDL2.dll

clean:
	find src -type f -name "*.o" -delete
	rm -f $(EXECUTABLE)

lib_lua:
	mkdir -p extern
	mkdir -p temp
	curl -L -o temp/temp.tar.gz https://www.lua.org/ftp/lua-5.4.8.tar.gz
	tar -xzf temp/temp.tar.gz -C extern
	rm -rf temp

	cd extern/lua-5.4.8 && make all PLAT=$(PLAT)

lib_sdl2:
ifeq ($(PLAT),mingw)
	mkdir -p extern
	mkdir -p temp
	curl -L -o temp/temp.zip https://github.com/libsdl-org/SDL/releases/download/release-2.32.8/SDL2-devel-2.32.8-mingw.zip
	unzip -o temp/temp.zip -d temp
	rm temp/temp.zip

	mkdir -p extern/sdl2
	cp -r temp/SDL2-2.32.8/x86_64-w64-mingw32/include extern/sdl2
	cp -r temp/SDL2-2.32.8/x86_64-w64-mingw32/lib extern/sdl2
	cp temp/SDL2-2.32.8/x86_64-w64-mingw32/bin/SDL2.dll .
	rm -rf temp
endif
# TODO: add macosx support

# TODO: finish this rule
lib_stb_image:
	mkdir -p extern
	mkdir -p extern/stb
	curl -L -o extern/stb/stb_image.h https://raw.githubusercontent.com/nothings/stb/refs/heads/master/stb_image.h