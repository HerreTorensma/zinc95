/*
SDL2 wrapper
*/

#pragma once

#include <SDL2/SDL.h>
#include <stdbool.h>

#include "../computer.h"
#include "input.h"
#include "gfx.h"

void sdl2_init(char title[], int initial_scale);

bool sdl2_window_is_open();

void sdl2_get_mouse_pos(int *x, int *y);

void sdl2_tick_start(computer_t *computer);

void sdl2_render(computer_t *computer);

void sdl2_tick_end();

void sdl2_quit();

void sdl2_input_update();

bool sdl2_input_key_pressed(zinc_key_t key);

bool sdl2_input_key_held(zinc_key_t key);

bool sdl2_input_key_released(zinc_key_t key);

bool sdl2_input_mouse_button_pressed(mouse_button_t gui_button);

bool sdl2_input_mouse_button_released(mouse_button_t gui_button);

bool sdl2_input_mouse_button_held(mouse_button_t gui_button);

bool sdl2_input_mouse_scrolled(scroll_dir_t direction);

// void sdl2_audio_callback(void *userdata, uint8_t *stream, int len);
void sdl2_audio_init(computer_t *computer);

void sdl2_set_clipboard_text(char *text);

string_t sdl2_get_clipboard_text(allocator_t allocator);

void sdl2_load_bmp_to_surface(palette_t *palette, surface_t surface, string_t path);

void sdl2_set_cursor_style(cursor_style_t style);
