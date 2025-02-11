#pragma once

#include <stdbool.h>

#include "../computer.h"

void backend_init(char title[], int initial_scale);

void backend_tick(computer_t *computer);

void backend_render(computer_t *computer);

void backend_tick_end(computer_t *computer);

bool window_is_open();

void get_mouse_pos(int *x, int *y);

void backend_quit();