#include <stdbool.h>
#include <stdio.h>

#include "backend.h"
#include "sdl2.h"
#include "../computer.h"

#define BACKEND_SDL2

void backend_init(char title[], int initial_scale) {
	#ifdef BACKEND_SDL2
	sdl2_init(title, initial_scale);
	#endif
}

void backend_tick(computer_t *computer) {
	#ifdef BACKEND_SDL2
	sdl2_tick(computer);
	#endif
}

void backend_render(computer_t *computer) {
	generate_rgb_framebuffer(computer);

	#ifdef BACKEND_SDL2
	sdl2_render(computer);
	#endif
}

void backend_tick_end(computer_t *computer) {
	#ifdef BACKEND_SDL2
	sdl2_tick_end();
	#endif
}

bool window_is_open() {
	#ifdef BACKEND_SDL2
	return sdl2_window_is_open();
	#endif
}

void backend_quit() {
	#ifdef BACKEND_SDL2
	sdl2_quit();
	#endif
}