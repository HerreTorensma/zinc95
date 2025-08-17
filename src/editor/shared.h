/*
Shared components among editors
*/

#pragma once

#include "../computer.h"
#include "../common/math2d.h"

typedef enum sprite_select_snap_mode {
	SNAP_MODE_SPRITE,
	SNAP_MODE_ZOOM,
	SNAP_MODE_FREE,
} sprite_select_snap_mode_t;

void sprite_selector_init(computer_t *computer);

void sprite_selector_update(computer_t *computer, sprite_select_snap_mode_t snap_mode, point_t pos);

void sprite_selector_draw(computer_t *computer, point_t pos, point_t page_buttons_pos);

// Get the rect of the current page
rect_t get_page_rect();

// Selected rect
rect_t get_in_frame_rect();

// Selected rect in number of sprites, like for example 
rect_t get_in_frame_rect_in_sprites();

int get_page_index();

int get_relative_sprite_index();

int get_absolute_sprite_index();

int sprite_coords_to_index(int x, int y);

// Translates point within selected rect of sprite selector to point within spritesheet
point_t editor_to_spritesheet_pos(point_t point);
