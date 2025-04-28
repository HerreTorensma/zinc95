#include "input.h"
#include "sdl2.h"

bool input_key_pressed(zinc_key_t key) {
	#ifdef BACKEND_SDL2
	return sdl2_input_key_pressed(key);
	#endif
}

bool input_key_held(zinc_key_t key) {
	#ifdef BACKEND_SDL2
	return sdl2_input_key_held(key);
	#endif
}

bool input_key_released(zinc_key_t key) {
	#ifdef BACKEND_SDL2
	return sdl2_input_key_released(key);
	#endif
}

bool input_mouse_button_pressed(mouse_button_t gui_button) {
	#ifdef BACKEND_SDL2
	return sdl2_input_mouse_button_pressed(gui_button);
	#endif
}

bool input_mouse_button_released(mouse_button_t gui_button) {
	#ifdef BACKEND_SDL2
	return sdl2_input_mouse_button_released(gui_button);
	#endif
}

bool input_mouse_button_held(mouse_button_t gui_button) {
	#ifdef BACKEND_SDL2
	return sdl2_input_mouse_button_held(gui_button);
	#endif
}

bool input_mouse_scrolled(scroll_dir_t direction) {
	#ifdef BACKEND_SDL2
	return sdl2_input_mouse_scrolled(direction);
	#endif
}

vec2i_t input_get_mouse_pos() {
	#ifdef BACKEND_SDL2

	int x, y;
	sdl2_get_mouse_pos(&x, &y);
	return (vec2i_t){x, y};

	#endif
}