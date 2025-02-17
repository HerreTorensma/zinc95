#pragma once

#include "../computer.h"

void load_palette_from_disk(computer_t *computer, const char filename[]);

// Draw inside the specified area
void draw_out_frame(computer_t *computer, rect_t rect);

// Draw around the specified area
void draw_in_frame(computer_t *computer, rect_t rect);