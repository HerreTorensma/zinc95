/*
GUI

GUI-related stuff like text drawing, buttons etc.
*/

#pragma once

#include "../computer.h"
#include "../backend/math2d.h"

void gui_draw_text(computer_t *computer, int font_index, char text[], vec2i_t pos, int color);

// Draw inside the specified area
void gui_outset_frame(computer_t *computer, recti_t rect);

// Draw around the specified area
void gui_inset_frame(computer_t *computer, recti_t rect);

bool gui_button_ex(computer_t *computer, char text[], recti_t rect, bool already_pressed);

bool gui_button(computer_t *computer, char text[], recti_t rect);

bool gui_press_button(computer_t *computer, char text[], recti_t rect);

// TODO: get rid of the bool pointer
bool gui_toggle_button(computer_t *computer, char text[], recti_t rect, bool *pressed);

int gui_get_text_width(font_t *font, char text[], int max_offset);

int gui_x_to_text_index(font_t *font, char text[], int x);