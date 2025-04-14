#pragma once

#include "../computer.h"

extern rect_t spritesheet_rect;
extern rect_t visible_rect;
extern rect_t currently_editing_rect;

extern int selected_sprite_index_offset;
extern int selected_spritesheet_index;

void sprite_selector_init(computer_t *computer);

void sprite_selector_update(computer_t *computer);

void sprite_selector_draw(computer_t *computer);

int get_selected_sprite_index();
