/*
GUI

GUI-related stuff like text drawing, buttons etc.
*/

#pragma once

#include "../computer.h"
#include "../common/math2d.h"
#include "../common/string.h"
#include "gfx.h"

#define GUI_STANDARD_BUTTON_SIZE 16
#define GUI_SMALL_BUTTON_SIZE 12
#define GUI_BORDER_WIDTH 2

typedef struct gui_knob_state {
	bool held;
	int64_t value_when_pressed;
} gui_knob_state_t;

void gui_draw_string(ram_t *ram, int font_index, string_t string, point_t pos, int color);

void gui_draw_text(ram_t *ram, int font_index, const char text[], point_t pos, int color);

bool gui_button(ram_t *ram, point_t pos, button_t button, bool already_pressed);

bool gui_press_button(ram_t *ram, point_t pos, button_t button);

bool gui_toggle_button(ram_t *ram, point_t pos, button_t button, bool set);

void gui_init_monospace_font_widths(ram_t *ram, int font_index, int width);

void gui_init_font_widths(ram_t *ram, int font_index);

rect_t gui_rect_to_outset_frame_rect(rect_t rect);

button_t button_array_get(button_array_t *array, int index);

point_t button_array_get_pos(button_array_t *array, point_t base_pos, int index);

void gui_load_skin(ram_t *ram, string_t path, color_t color_key, color_t font_color);

// int64_t gui_knob(ram_t *ram, int font_index, rect_t rect, int64_t min, int64_t max, int64_t value, gui_knob_state_t *state);
int64_t gui_knob(ram_t *ram, int font_index, point_t center, int radius, point_t text_pos, int64_t min, int64_t max, int64_t value, gui_knob_state_t *state);

void gui_draw_selection_rect(uint64_t ticks, surface_t surf, rect_t rect);

int gui_button_matrix(ram_t *ram, point_t pos, button_matrix_t matrix, int already_pressed_index);
