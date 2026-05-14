/*
Shared components among editors
*/

#pragma once

#include "../computer.h"
#include "../common/math2d.h"

void sprite_selector_init(computer_t *computer);

void sprite_selector_zoom_in();

void sprite_selector_zoom_out();

void sprite_selector_update(computer_t *computer, point_t pos);

void sprite_selector_draw(computer_t *computer, point_t pos, point_t page_buttons_pos, point_t snap_mode_buttons_pos);

// Get the rect of the current page
rect_t get_page_rect();

// Selected rect
rect_t get_in_frame_rect();

// Selected rect in number of sprites, like for example 
rect_t get_in_frame_rect_in_sprites();

int get_sprite_index();

int sprite_coords_to_index(int x, int y);
