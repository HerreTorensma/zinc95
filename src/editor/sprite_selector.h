#pragma once

#include "../computer.h"

void sprite_selector_init(computer_t *computer);

void sprite_selector_update(computer_t *computer);

void sprite_selector_draw(computer_t *computer);

void sprite_editor_set_selected_spritesheet_index(int index);
