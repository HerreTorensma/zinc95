#pragma once

#include <SDL2/SDL.h>
#include <stdbool.h>

#include "../computer.h"

void sdl2_init(char title[], int initial_scale);

bool sdl2_window_is_open();

void sdl2_tick(computer_t *computer);

void sdl2_render(computer_t *computer);

void sdl2_tick_end();

void sdl2_quit();