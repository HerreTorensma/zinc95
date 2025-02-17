#include "input.h"
#include "sdl2.h"

bool input_key_pressed(key_t key) {
	#ifdef BACKEND_SDL2
	return sdl2_input_key_pressed(key);
	#endif
}

bool input_key_held(key_t key) {
	#ifdef BACKEND_SDL2
	return sdl2_input_key_held(key);
	#endif
}

bool input_key_released(key_t key) {
	#ifdef BACKEND_SDL2
	return sdl2_input_key_released(key);
	#endif
}

bool input_mouse_button_pressed(mouse_button_t button) {
	#ifdef BACKEND_SDL2
	return sdl2_input_mouse_button_pressed(button);
	#endif
}

bool input_mouse_button_released(mouse_button_t button) {
	#ifdef BACKEND_SDL2
	return sdl2_input_mouse_button_released(button);
	#endif
}

bool input_mouse_button_held(mouse_button_t button) {
	#ifdef BACKEND_SDL2
	return sdl2_input_mouse_button_held(button);
	#endif
}