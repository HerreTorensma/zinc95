#pragma once

#include <SDL2/SDL.h>
#include <stdbool.h>

#include "../computer.h"
#include "input.h"

void sdl2_init(char title[], int initial_scale);

bool sdl2_window_is_open();

void sdl2_get_mouse_pos(int *x, int *y);

void sdl2_tick(computer_t *computer);

void sdl2_render(computer_t *computer);

void sdl2_tick_end();

void sdl2_quit();

void sdl2_input_update();

bool sdl2_input_key_pressed(key_t key);

bool sdl2_input_key_held(key_t key);

bool sdl2_input_key_released(key_t key);

bool sdl2_input_mouse_button_pressed(mouse_button_t button);

bool sdl2_input_mouse_button_released(mouse_button_t button);

bool sdl2_input_mouse_button_held(mouse_button_t button);