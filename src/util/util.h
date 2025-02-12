#pragma once

#include "../computer.h"

void load_palette_from_disk(computer_t *computer, const char filename[]);

// Draw inside the specified area
void draw_out_frame(computer_t *computer, int x, int y, int w, int h);

// Draw around the specified area
void draw_in_frame(computer_t *computer, int x, int y, int w, int h);