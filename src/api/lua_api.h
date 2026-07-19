/*
Lua API
*/

#pragma once

#include "../computer.h"

int lua_init(computer_t *computer);

int lua_call_init();

int lua_call_update();

int lua_call_draw();

void lua_quit();

// string_t serialize_entities(allocator_t allocator, ram_t *ram);
