/*
GUI

GUI-related stuff like text drawing, buttons etc.
*/

#pragma once

#include "../computer.h"
#include "../backend/math2d.h"
#include "../backend/gfx.h"

#define GUI_STANDARD_BUTTON_SIZE 16
#define GUI_SMALL_BUTTON_SIZE 12
#define GUI_BORDER_WIDTH 2

void gui_draw_string(ram_t *ram, int font_index, string_t string, point_t pos, int color);

void gui_draw_text(ram_t *ram, int font_index, const char text[], point_t pos, int color);

bool gui_button(ram_t *ram, point_t pos, button_t button, bool already_pressed);

bool gui_press_button(ram_t *ram, point_t pos, button_t button);

bool gui_toggle_button(ram_t *ram, point_t pos, button_t button, bool set);

int gui_get_string_width(font_t *font, string_t string, int max_offset);

int gui_get_text_width(font_t *font, char text[], int max_offset);

int gui_x_to_string_index(font_t *font, string_t string, int x);

void gui_init_monospace_font_widths(ram_t *ram, int font_index, int width);

rect_t gui_rect_to_outset_frame_rect(rect_t rect);

button_t button_array_get(button_array_t *array, int index);

point_t button_array_get_pos(button_array_t *array, point_t base_pos, int index);
