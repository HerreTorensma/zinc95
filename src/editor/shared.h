/*
Shared components among editors
*/

#pragma once

#include "../computer.h"
#include "../backend/math2d.h"

extern recti_t spritesheet_rect;
extern recti_t visible_rect;
extern recti_t currently_editing_rect;
extern recti_t currently_editing_sprites_rect;

extern int selected_sprite_index_offset;
extern int selected_spritesheet_index;

typedef enum sprite_select_snap_mode {
	SNAP_MODE_SPRITE,
	SNAP_MODE_ZOOM,
	SNAP_MODE_FREE,
} sprite_select_snap_mode_t;

void sprite_selector_init(computer_t *computer);

void sprite_selector_update(computer_t *computer, sprite_select_snap_mode_t snap_mode);

void sprite_selector_draw(computer_t *computer);

int get_selected_sprite_index();

int sprite_coords_to_index(int x, int y);