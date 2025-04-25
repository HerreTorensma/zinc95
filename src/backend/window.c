#include <stdbool.h>
#include <stdio.h>

#include "window.h"
#include "sdl2.h"
#include "gfx.h"
#include "../computer.h"

void window_init(char title[], int initial_scale) {
	#ifdef BACKEND_SDL2
	sdl2_init(title, initial_scale);
	#endif
}

void window_tick_start(computer_t *computer) {
	#ifdef BACKEND_SDL2
	sdl2_tick_start(computer);
	#endif
}

void window_render(computer_t *computer) {
	gfx_generate_rgb_framebuffer(computer);

	#ifdef BACKEND_SDL2
	sdl2_render(computer);
	#endif
}

void window_tick_end(computer_t *computer) {
	#ifdef BACKEND_SDL2
	sdl2_tick_end();
	#endif

	computer->ticks++;
}

bool window_is_open() {
	#ifdef BACKEND_SDL2
	return sdl2_window_is_open();
	#endif
}

void window_quit() {
	#ifdef BACKEND_SDL2
	sdl2_quit();
	#endif
}