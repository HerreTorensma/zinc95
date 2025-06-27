#pragma once

#include "../computer.h"

void txt_generate_rgb_framebuffer(computer_t *computer);

// Low level textbuffer manipulation
void txt_putchar(ram_t *ram, uint8_t c, int x, int y, color_t fg_color, color_t bg_color);

void txt_clear(ram_t *ram);

// Middle level terminal manipulation

// High level shell manipulation