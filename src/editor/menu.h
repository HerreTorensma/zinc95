/*
Editor menu
Draws bar and is in charge of calling the correct editor update and draw loop
*/

#pragma once

#include "../computer.h"
#include "../common/math2d.h"

typedef enum workspace_type {
	WORKSPACE_CODE,
	WORKSPACE_SPRITE,
	WORKSPACE_MAP,
	WORKSPACE_SOUND,
	WORKSPACE_MUSIC,
} workspace_type_t;

void switch_to_workspace(size_t index);

void workspace_menu_init(computer_t *computer);

void workspace_menu_update(computer_t *computer);

void workspace_menu_draw(computer_t *computer);

void workspace_menu_deinit(computer_t *computer);

void push_log(ram_t *ram, string_t message);
