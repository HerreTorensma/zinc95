/*
Sound editor
*/

#pragma once

#include "../computer.h"

void sound_set_current_pattern(size_t index);

void sound_editor_init(computer_t *computer);

void sound_editor_update(computer_t *computer);

void sound_editor_draw(computer_t *computer);
