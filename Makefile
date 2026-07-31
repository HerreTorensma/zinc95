CC = gcc
CFLAGS = -O2 -g -Wall -Wextra -Wsign-conversion -Wpedantic -std=c11 -Wno-unused-parameter -Werror=incompatible-pointer-types -funsigned-char
EXECUTABLE = zinc95

# Source files
SRC = src/main.c src/computer.c src/serialize.c \
src/core/window_sdl2.c src/core/input.c src/core/input_sdl2.c src/core/gfx.c src/core/gfx_sdl2.c src/core/file.c src/core/audio.c src/core/audio_sdl2.c src/core/txt.c \
src/editor/gui.c src/editor/menu.c  src/editor/code.c src/editor/sprite.c src/editor/map.c \
src/editor/sound.c src/editor/music.c src/editor/shared.c \
src/api/api.c src/api/lua_api.c \
src/common/math2d.c src/common/mem.c src/common/string.c src/common/io.c src/common/serialize.c src/common/serialize_zlib.c \
src/tests.c

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
	SRC += src/common/io_windows.c
	CFLAGS += -Iextern/sdl2/include -Iextern/lua-5.4.8/src
	LDFLAGS = extern/lua-5.4.8/src/liblua.a -Lextern/sdl2/lib -lmingw32 -lSDL2main -lSDL2 -lz -lm
	EXECUTABLE = zinc95.exe
endif

ifeq ($(PLAT),linux)
	SRC += src/common/io_posix.c
	CFLAGS += -Iextern/lua-5.4.8/src
	LDFLAGS = extern/lua-5.4.8/src/liblua.a -lSDL2 -lz -lm
endif

ifeq ($(PLAT),macosx)
	SRC += src/common/io_posix.c
	CFLAGS += -Iextern/sdl2/include -Iextern/lua-5.4.8/src -DPLATFORM_MACOSX
	LDFLAGS = extern/lua-5.4.8/src/liblua.a -Lextern/sdl2/lib -lSDL2 -lz -lm
endif

all: build

libs: lib_lua lib_sdl2

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

# TODO: finish windows support
# lib_zlib:
# 	mkdir -p extern
# 	mkdir -p temp
# 	curl -L -o temp/temp.tar.gz https://github.com/madler/zlib/releases/download/v1.3.2/zlib-1.3.2.tar.gz
# 	tar -xzf temp/temp.tar.gz -C extern
# 	rm -rf temp
