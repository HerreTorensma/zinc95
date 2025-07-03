/*
Editor menu
Draws bar and is in charge of calling the correct editor update and draw loop
*/

#pragma once

#include "../computer.h"
#include "../common/math2d.h"

void workspace_menu_init(computer_t *computer);

void workspace_menu_update(computer_t *computer);

void workspace_menu_draw(computer_t *computer);