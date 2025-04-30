/*
Input
*/

#pragma once

#include <stdbool.h>

#include "math2d.h"

// TODO: manually number these (maybe)
// Also write them to RAM
typedef enum zinc_key {
	KEY_UNKNOWN = 0,

	KEY_A,
	KEY_B,
	KEY_C,
	KEY_D,
	KEY_E,
	KEY_F,
	KEY_G,
	KEY_H,
	KEY_I,
	KEY_J,
	KEY_K,
	KEY_L,
	KEY_M,
	KEY_N,
	KEY_O,
	KEY_P,
	KEY_Q,
	KEY_R,
	KEY_S,
	KEY_T,
	KEY_U,
	KEY_V,
	KEY_W,
	KEY_X,
	KEY_Y,
	KEY_Z,
	
	KEY_0,
	KEY_1,
	KEY_2,
	KEY_3,
	KEY_4,
	KEY_5,
	KEY_6,
	KEY_7,
	KEY_8,
	KEY_9,

	KEY_MINUS,
	KEY_EQUALS,
	
	KEY_LEFTBRACKET,
	KEY_RIGHTBRACKET,
	KEY_BACKSLASH,
	KEY_SEMICOLON,
	KEY_APOSTROPHE,
	KEY_GRAVE,
	KEY_COMMA,
	KEY_PERIOD,
	KEY_SLASH,

	KEY_SPACE,
	KEY_TAB,
	KEY_RETURN,
	KEY_BACKSPACE,
	KEY_DELETE,
	KEY_INSERT,
	KEY_PAGEUP,
	KEY_PAGEDOWN,
	KEY_HOME,
	KEY_END,
	KEY_UP,
	KEY_DOWN,
	KEY_LEFT,
	KEY_RIGHT,
	KEY_CAPSLOCK,

	KEY_LCTRL,
	KEY_LSHIFT,
	KEY_LALT,
	KEY_RCTRL,
	KEY_RSHIFT,
	KEY_RALT,
	
	KEY_ESC,
	KEY_F1,
	KEY_F2,
	KEY_F3,
	KEY_F4,
	KEY_F5,
	KEY_F6,
	KEY_F7,
	KEY_F8,
	KEY_F9,
	KEY_F10,
	KEY_F11,
	KEY_F12,

	KEY_NUM0,
	KEY_NUM1,
	KEY_NUM2,
	KEY_NUM3,
	KEY_NUM4,
	KEY_NUM5,
	KEY_NUM6,
	KEY_NUM7,
	KEY_NUM8,
	KEY_NUM9,
	KEY_NUMPLUS,
	KEY_NUMMINUS,
	KEY_NUMMULTIPLY,
	KEY_NUMDIVIDE,
	KEY_NUMENTER,
	KEY_NUMPERIOD,
} zinc_key_t;

typedef enum mouse_button {
	MOUSE_BUTTON_UNKNOWN = 0,
	MOUSE_BUTTON_LEFT = 1,
	MOUSE_BUTTON_MIDDLE = 2,
	MOUSE_BUTTON_RIGHT = 3,
} mouse_button_t;

typedef enum scroll_dir {
	SCROLL_DIR_UP = -1,
	SCROLL_DIR_NONE = 0,
	SCROLL_DIR_DOWN = 1,
} scroll_dir_t;

// TODO: rename this to make it more clear they're checks
// like check_keyp or something
bool input_key_pressed(zinc_key_t key);

bool input_key_held(zinc_key_t key);

bool input_key_released(zinc_key_t key);

bool input_mouse_button_pressed(mouse_button_t button);

bool input_mouse_button_released(mouse_button_t button);

bool input_mouse_button_held(mouse_button_t button);

bool input_mouse_scrolled(scroll_dir_t direction);

point_t input_get_mouse_pos();