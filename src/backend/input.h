#pragma once

#include <SDL2/SDL.h>
#include <stdbool.h>

typedef enum key {
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

	KEY_1,
	KEY_2,
	KEY_3,
	KEY_4,
	KEY_5,
	KEY_6,
	KEY_7,
	KEY_8,
	KEY_9,
	KEY_0,
} key_t;

typedef enum mouse_button {
	MOUSE_BUTTON_UNKNOWN = 0,
	MOUSE_BUTTON_LEFT = 1,
	MOUSE_BUTTON_MIDDLE = 2,
	MOUSE_BUTTON_RIGHT = 3,
} mouse_button_t;

bool input_key_pressed(key_t key);

bool input_key_held(key_t key);

bool input_key_released(key_t key);

bool input_mouse_button_pressed(mouse_button_t button);

bool input_mouse_button_released(mouse_button_t button);

bool input_mouse_button_held(mouse_button_t button);