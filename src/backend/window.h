/*
Wrapper around windowing library
*/

#pragma once

#include <stdbool.h>

#include "../computer.h"

void window_init(char title[], int initial_scale);

void window_tick_start(computer_t *computer);

void window_render(computer_t *computer);

void window_tick_end(computer_t *computer);

bool window_is_open();

void get_mouse_pos(int *x, int *y);

void window_quit();