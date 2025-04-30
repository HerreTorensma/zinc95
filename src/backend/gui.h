/*
GUI

GUI-related stuff like text drawing, buttons etc.
*/

#pragma once

#include "../computer.h"
#include "../backend/math2d.h"

void gui_draw_text(ram_t *ram, int font_index, char text[], point_t pos, int color);

// Draw inside the specified area
void gui_outset_frame(ram_t *ram, rect_t rect);

// Draw around the specified area
void gui_inset_frame(ram_t *ram, rect_t rect);

bool gui_button_ex(ram_t *ram, char text[], rect_t rect, bool already_pressed);

bool gui_button(ram_t *ram, char text[], rect_t rect);

bool gui_press_button(ram_t *ram, char text[], rect_t rect);

bool gui_toggle_button(ram_t *ram, char text[], rect_t rect, bool set);

int gui_get_text_width(font_t *font, char text[], int max_offset);

int gui_x_to_text_index(font_t *font, char text[], int x);