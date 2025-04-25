/*
Menu editor
*/

#pragma once

#include "../computer.h"
#include "../backend/math2d.h"

extern recti_t workspace_rect;

void workspace_menu_init(computer_t *computer);

void workspace_menu_update(computer_t *computer);

void workspace_menu_draw(computer_t *computer);