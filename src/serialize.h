#pragma once

#include "computer.h"

void game_save(const computer_t *computer, const string_t path);

int game_load(computer_t *computer, string_t path);
