/*
General utilities
*/

#pragma once

#include "../computer.h"
#include "../backend/math2d.h"

// Draw inside the specified area
void draw_out_frame(computer_t *computer, recti_t rect);

// Draw around the specified area
void draw_in_frame(computer_t *computer, recti_t rect);

bool button_ex(computer_t *computer, char text[], recti_t rect, bool appear_pressed);

bool button(computer_t *computer, char text[], recti_t rect);

bool press_button(computer_t *computer, char text[], recti_t rect);

// TODO: get rid of the bool pointer
bool toggle_button(computer_t *computer, char text[], recti_t rect, bool *pressed);

// recti_t sprite_index_to_spritesheet_rect(ram_t *ram, int sprite_index, int w, int h);