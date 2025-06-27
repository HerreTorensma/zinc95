#include "txt.h"

#include <string.h>

// Directly generates the rgb framebuffer from the textbuffer
void txt_generate_rgb_framebuffer(computer_t *computer) {
	for (int y = 0; y < TEXTBUFFER_HEIGHT; y++) {
		for (int x = 0; x < TEXTBUFFER_WIDTH; x++) {
			char_t character = computer->ram->textbuffer.data[y * TEXTBUFFER_WIDTH + x];

			point_t font_cell = POINT(character.c % 16, character.c / 16); // TODO: not hardcode

			point_t pos_in_font = POINT(font_cell.x * TEXTBUFFER_CHAR_WIDTH, font_cell.y * TEXTBUFFER_CHAR_HEIGHT);
			point_t pos_on_screen = POINT(x * TEXTBUFFER_CHAR_WIDTH, y * TEXTBUFFER_CHAR_HEIGHT);
			
			for (int i = 0; i < TEXTBUFFER_CHAR_HEIGHT; i++) {
				for (int j = 0; j < TEXTBUFFER_CHAR_WIDTH; j++) {
					// Pixel in the character
					color_t pixel = computer->ram->text_mode_font.data[(pos_in_font.y + i) * TEXT_MODE_FONT_BITMAP_WIDTH + (pos_in_font.x + j)];

					if (pixel == COLOR_BLACK) {
						computer->rgb_framebuffer[(pos_on_screen.y + i) * SCREEN_WIDTH + (pos_on_screen.x + j)] = computer->ram->palette.colors[character.bg_color];
					} else if (pixel == COLOR_WHITE) {
						computer->rgb_framebuffer[(pos_on_screen.y + i) * SCREEN_WIDTH + (pos_on_screen.x + j)] = computer->ram->palette.colors[character.fg_color];
					}
				}
			}
		}
	}
}

void txt_putchar(ram_t *ram, uint8_t c, int x, int y, color_t fg_color, color_t bg_color) {
	if (x < 0 || x >= TEXTBUFFER_WIDTH)
		return;

	if (y < 0 || y >= TEXTBUFFER_HEIGHT)
		return;

	ram->textbuffer.data[y * TEXTBUFFER_WIDTH + x] = (char_t){
		.c = c,
		.bg_color = bg_color,
		.fg_color = fg_color,
	};
}

// TODO: test
void txt_clear(ram_t *ram) {
	memset(ram->textbuffer.data, 0, TEXTBUFFER_WIDTH * TEXTBUFFER_HEIGHT * sizeof(char_t));
}

void term_putchar(uint8_t c);

void term_print(string_t string);