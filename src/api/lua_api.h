/*
Lua API
*/

#pragma once

#include "../computer.h"

void lua_init(computer_t *computer);

void lua_call_init();

void lua_call_update();

void lua_call_draw();

void lua_quit();