/*
Textbuffer, terminal and shell related functionality
*/

#pragma once

#include "../computer.h"
#include "../common/string.h"

void txt_generate_rgb_framebuffer(computer_t *computer);

// Low level textbuffer manipulation
void txt_putchar(ram_t *ram, uint8_t c, int x, int y, color_t bg_color, color_t fg_color);

void txt_clear(ram_t *ram);

void txt_shift_lines_up(ram_t *ram);

void txt_shift_lines_down(ram_t *ram);

// Middle level terminal manipulation
void term_putchar(ram_t *ram, uint8_t c, color_t bg_color, color_t fg_color);

void term_printc(ram_t *ram, string_t string, color_t bg_color, color_t fg_color);

void term_print(ram_t *ram, string_t string);

char term_getchar();

// High level shell manipulation
void shell_init(computer_t *computer);

void shell_update(computer_t *computer);

void shell_new_command(computer_t *computer);

void shell_deinit(computer_t *computer);
