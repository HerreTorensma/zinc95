#include <stdbool.h>
#include <stdio.h>

#include "window.h"
#include "sdl2.h"
#include "gfx.h"
#include "txt.h"
#include "../computer.h"

// TODO: instead of the ifdef stuff SDL2 there should be one window.h file and then window_sdl2.c or window_sokol.c what contain the implementation
// because this is almost that but with more bloat
// I will take care of this in another commit
// Like the IO

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
	// if (computer->ram->video_mode == VIDEO_MODE_GRAPHICS) {
	// 	gfx_generate_rgb_framebuffer(computer);
	// } else if (computer->ram->video_mode == VIDEO_MODE_TEXT) {
	// 	txt_generate_rgb_framebuffer(computer);
	// }

	#ifdef BACKEND_SDL2
	sdl2_render(computer);
	#endif
}

void window_tick_end(computer_t *computer) {
	#ifdef BACKEND_SDL2
	sdl2_tick_end();
	#endif

	computer->ram->ticks++;
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

void set_clipboard_text(string_t string) {
	sdl2_set_clipboard_text(string.data);
}
