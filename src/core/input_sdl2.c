#include "input.h"

#include <SDL2/SDL.h>

typedef struct sdl2_input {
	uint8_t prev_key_state[512];
	uint8_t key_state[512];

	uint32_t prev_mouse_state;
	uint32_t mouse_state;
} sdl2_input_t;

static sdl2_input_t _sdl2_input = {0};

static SDL_Cursor *_cursors[CURSOR_STYLE_COUNT];

// -1 is up, 0 is none, 1 is down
static int _scroll_state = 0;

static SDL_Scancode _key_to_sdl2_scancode(zinc_key_t key) {
	// Use command key on MacOS
	#ifdef PLATFORM_MACOSX
	if (key == KEY_LCTRL) {
		return SDL_SCANCODE_LGUI;
	}
	if (key == KEY_RCTRL) {
		return SDL_SCANCODE_RGUI;
	}
	#endif

	switch (key) {
		case KEY_A: return SDL_SCANCODE_A;
		case KEY_B: return SDL_SCANCODE_B;
		case KEY_C: return SDL_SCANCODE_C;
		case KEY_D: return SDL_SCANCODE_D;
		case KEY_E: return SDL_SCANCODE_E;
		case KEY_F: return SDL_SCANCODE_F;
		case KEY_G: return SDL_SCANCODE_G;
		case KEY_H: return SDL_SCANCODE_H;
		case KEY_I: return SDL_SCANCODE_I;
		case KEY_J: return SDL_SCANCODE_J;
		case KEY_K: return SDL_SCANCODE_K;
		case KEY_L: return SDL_SCANCODE_L;
		case KEY_M: return SDL_SCANCODE_M;
		case KEY_N: return SDL_SCANCODE_N;
		case KEY_O: return SDL_SCANCODE_O;
		case KEY_P: return SDL_SCANCODE_P;
		case KEY_Q: return SDL_SCANCODE_Q;
		case KEY_R: return SDL_SCANCODE_R;
		case KEY_S: return SDL_SCANCODE_S;
		case KEY_T: return SDL_SCANCODE_T;
		case KEY_U: return SDL_SCANCODE_U;
		case KEY_V: return SDL_SCANCODE_V;
		case KEY_W: return SDL_SCANCODE_W;
		case KEY_X: return SDL_SCANCODE_X;
		case KEY_Y: return SDL_SCANCODE_Y;
		case KEY_Z: return SDL_SCANCODE_Z;

		case KEY_0: return SDL_SCANCODE_0;
		case KEY_1: return SDL_SCANCODE_1;
		case KEY_2: return SDL_SCANCODE_2;
		case KEY_3: return SDL_SCANCODE_3;
		case KEY_4: return SDL_SCANCODE_4;
		case KEY_5: return SDL_SCANCODE_5;
		case KEY_6: return SDL_SCANCODE_6;
		case KEY_7: return SDL_SCANCODE_7;
		case KEY_8: return SDL_SCANCODE_8;
		case KEY_9: return SDL_SCANCODE_9;

		case KEY_MINUS: return SDL_SCANCODE_MINUS;
		case KEY_EQUALS: return SDL_SCANCODE_EQUALS;
		case KEY_LEFTBRACKET: return SDL_SCANCODE_LEFTBRACKET;
		case KEY_RIGHTBRACKET: return SDL_SCANCODE_RIGHTBRACKET;
		case KEY_BACKSLASH: return SDL_SCANCODE_BACKSLASH;
		case KEY_SEMICOLON: return SDL_SCANCODE_SEMICOLON;
		case KEY_APOSTROPHE: return SDL_SCANCODE_APOSTROPHE;
		case KEY_GRAVE: return SDL_SCANCODE_GRAVE;
		case KEY_COMMA: return SDL_SCANCODE_COMMA;
		case KEY_PERIOD: return SDL_SCANCODE_PERIOD;
		case KEY_SLASH: return SDL_SCANCODE_SLASH;

		case KEY_SPACE: return SDL_SCANCODE_SPACE;
		case KEY_TAB: return SDL_SCANCODE_TAB;
		case KEY_RETURN: return SDL_SCANCODE_RETURN;
		case KEY_BACKSPACE: return SDL_SCANCODE_BACKSPACE;
		case KEY_DELETE: return SDL_SCANCODE_DELETE;
		case KEY_INSERT: return SDL_SCANCODE_INSERT;
		case KEY_PAGEUP: return SDL_SCANCODE_PAGEUP;
		case KEY_PAGEDOWN: return SDL_SCANCODE_PAGEDOWN;
		case KEY_HOME: return SDL_SCANCODE_HOME;
		case KEY_END: return SDL_SCANCODE_END;
		case KEY_UP: return SDL_SCANCODE_UP;
		case KEY_DOWN: return SDL_SCANCODE_DOWN;
		case KEY_LEFT: return SDL_SCANCODE_LEFT;
		case KEY_RIGHT: return SDL_SCANCODE_RIGHT;
		case KEY_CAPSLOCK: return SDL_SCANCODE_CAPSLOCK;

		case KEY_LCTRL: return SDL_SCANCODE_LCTRL;
		case KEY_LSHIFT: return SDL_SCANCODE_LSHIFT;
		case KEY_LALT: return SDL_SCANCODE_LALT;
		case KEY_RCTRL: return SDL_SCANCODE_RCTRL;
		case KEY_RSHIFT: return SDL_SCANCODE_RSHIFT;
		case KEY_RALT: return SDL_SCANCODE_RALT;

		case KEY_ESC: return SDL_SCANCODE_ESCAPE;
		case KEY_F1: return SDL_SCANCODE_F1;
		case KEY_F2: return SDL_SCANCODE_F2;
		case KEY_F3: return SDL_SCANCODE_F3;
		case KEY_F4: return SDL_SCANCODE_F4;
		case KEY_F5: return SDL_SCANCODE_F5;
		case KEY_F6: return SDL_SCANCODE_F6;
		case KEY_F7: return SDL_SCANCODE_F7;
		case KEY_F8: return SDL_SCANCODE_F8;
		case KEY_F9: return SDL_SCANCODE_F9;
		case KEY_F10: return SDL_SCANCODE_F10;
		case KEY_F11: return SDL_SCANCODE_F11;
		case KEY_F12: return SDL_SCANCODE_F12;

		case KEY_NUM0: return SDL_SCANCODE_KP_0;
		case KEY_NUM1: return SDL_SCANCODE_KP_1;
		case KEY_NUM2: return SDL_SCANCODE_KP_2;
		case KEY_NUM3: return SDL_SCANCODE_KP_3;
		case KEY_NUM4: return SDL_SCANCODE_KP_4;
		case KEY_NUM5: return SDL_SCANCODE_KP_5;
		case KEY_NUM6: return SDL_SCANCODE_KP_6;
		case KEY_NUM7: return SDL_SCANCODE_KP_7;
		case KEY_NUM8: return SDL_SCANCODE_KP_8;
		case KEY_NUM9: return SDL_SCANCODE_KP_9;
		case KEY_NUMPLUS: return SDL_SCANCODE_KP_PLUS;
		case KEY_NUMMINUS: return SDL_SCANCODE_KP_MINUS;
		case KEY_NUMMULTIPLY: return SDL_SCANCODE_KP_MULTIPLY;
		case KEY_NUMDIVIDE: return SDL_SCANCODE_KP_DIVIDE;
		case KEY_NUMENTER: return SDL_SCANCODE_KP_ENTER;
		case KEY_NUMPERIOD: return SDL_SCANCODE_KP_PERIOD;

		default: return SDL_SCANCODE_UNKNOWN;
	}
}

static int _mouse_button_to_sdl2_button(mouse_button_t gui_button) {
	switch (gui_button) {
		case MOUSE_BUTTON_LEFT: return 1;
		case MOUSE_BUTTON_MIDDLE: return 2;
		case MOUSE_BUTTON_RIGHT: return 3;
		default: return 0;
	}
}

void input_init() {
	_cursors[CURSOR_STYLE_ARROW] = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_ARROW); 
	_cursors[CURSOR_STYLE_TEXT] = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_IBEAM); 
	_cursors[CURSOR_STYLE_MOVE] = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_SIZEALL); 
	_cursors[CURSOR_STYLE_HAND] = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_HAND); 
	_cursors[CURSOR_STYLE_CROSSHAIR] = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_CROSSHAIR);
}

void input_update() {
	memcpy(_sdl2_input.prev_key_state, _sdl2_input.key_state, 256 * sizeof(uint8_t));

	const uint8_t *state = SDL_GetKeyboardState(NULL);
	memcpy(_sdl2_input.key_state, state, 256 * sizeof(uint8_t));

	_sdl2_input.prev_mouse_state = _sdl2_input.mouse_state;
	
	int x, y;
	_sdl2_input.mouse_state = SDL_GetMouseState(&x, &y);

	_scroll_state = 0;
	SDL_Event event;
	while (SDL_PollEvent(&event)) {
		// Check scrolling
		if (event.type == SDL_MOUSEWHEEL) {
			if (event.wheel.y > 0) {
				_scroll_state = -1;
			} else if (event.wheel.y < 0) {
				_scroll_state = 1;
			} else {
				_scroll_state = 0;
			}
		}
	}
}

bool input_key_pressed(zinc_key_t key) {
	SDL_Scancode scancode = _key_to_sdl2_scancode(key);

	if (_sdl2_input.key_state[scancode] && !_sdl2_input.prev_key_state[scancode]) {
		return true;
	}
	return false;
}

bool input_key_held(zinc_key_t key) {
	SDL_Scancode scancode = _key_to_sdl2_scancode(key);

	if (_sdl2_input.key_state[scancode]) {
		return true;
	}
	return false;
}

bool input_key_released(zinc_key_t key) {
	SDL_Scancode scancode = _key_to_sdl2_scancode(key);

	if (!_sdl2_input.key_state[scancode] && _sdl2_input.prev_key_state[scancode]) {
		return true;
	}
	return false;
}

bool input_mouse_button_pressed(mouse_button_t gui_button) {
	int sdl2_button = _mouse_button_to_sdl2_button(gui_button);

	if (_sdl2_input.mouse_state & SDL_BUTTON(sdl2_button) && !(_sdl2_input.prev_mouse_state & SDL_BUTTON(sdl2_button))) {
		return true;
	}
	return false;
}

bool input_mouse_button_released(mouse_button_t gui_button) {
	int sdl2_button = _mouse_button_to_sdl2_button(gui_button);

	if (!(_sdl2_input.mouse_state & SDL_BUTTON(sdl2_button)) && _sdl2_input.prev_mouse_state & SDL_BUTTON(sdl2_button)) {
		return true;
	}
	return false;
}

bool input_mouse_button_held(mouse_button_t gui_button) {
	int sdl2_button = _mouse_button_to_sdl2_button(gui_button);

	if (_sdl2_input.mouse_state & SDL_BUTTON(sdl2_button)) {
		return true;
	}
	return false;
}

bool input_mouse_scrolled(scroll_dir_t direction) {
	return _scroll_state == direction;
}

void input_set_cursor_style(cursor_style_t style) {
	SDL_SetCursor(_cursors[style]);
}

point_t input_get_mouse_pos() {
	int sdl_x, sdl_y;
	SDL_GetMouseState(&sdl_x, &sdl_y);

	float adjusted_x = (float)sdl_x * _dpi_scale_x;
	float adjusted_y = (float)sdl_y * _dpi_scale_y;

	return (point_t){
		.x = (adjusted_x / _scale - _viewport_offset_x),
		.y = (adjusted_y / _scale - _viewport_offset_y),
	};
}
