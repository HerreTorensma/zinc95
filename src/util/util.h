#pragma once

#include "../computer.h"

// Draw inside the specified area
void draw_out_frame(computer_t *computer, rect_t rect);

// Draw around the specified area
void draw_in_frame(computer_t *computer, rect_t rect);

bool button_ex(computer_t *computer, char text[], rect_t rect, bool appear_pressed);

bool button(computer_t *computer, char text[], rect_t rect);

bool press_button(computer_t *computer, char text[], rect_t rect);