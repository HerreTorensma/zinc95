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

void txt_putchar(ram_t *ram, uint8_t c, int x, int y, color_t bg_color, color_t fg_color) {
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

void txt_shift_lines_up(ram_t *ram) {
	memmove(ram->textbuffer.data, ram->textbuffer.data + TEXTBUFFER_WIDTH, (TEXTBUFFER_SIZE - TEXTBUFFER_WIDTH) * sizeof(char_t));
	
	memset(ram->textbuffer.data + (TEXTBUFFER_SIZE - TEXTBUFFER_WIDTH), 0, TEXTBUFFER_WIDTH * sizeof(char_t));
}

void txt_shift_lines_down(ram_t *ram) {
	// Move the memory
	memmove(ram->textbuffer.data + TEXTBUFFER_WIDTH, ram->textbuffer.data, (TEXTBUFFER_SIZE - TEXTBUFFER_WIDTH) * sizeof(char_t));
	
	// The old memory still exists so set to 0
	memset(ram->textbuffer.data, 0, TEXTBUFFER_WIDTH * sizeof(char_t));
}

// TODO: test this
void txt_clear(ram_t *ram) {
	memset(ram->textbuffer.data, 0, TEXTBUFFER_SIZE * sizeof(char_t));
}

static size_t _get_cursor_index(terminal_t *terminal) {
	return terminal->cursor_y * TEXTBUFFER_WIDTH + terminal->cursor_x;
}

static void _scroll_if_needed(ram_t *ram) {
	if (ram->terminal.cursor_y >= TEXTBUFFER_HEIGHT) {
		txt_shift_lines_up(ram);
		ram->terminal.cursor_y = TEXTBUFFER_HEIGHT - 1;
	}
}

static void _cursor_newline(ram_t *ram) {
	ram->terminal.cursor_x = 0;
	ram->terminal.cursor_y++;

	_scroll_if_needed(ram);
}

static void _increment_cursor(ram_t *ram) {
	if (ram->terminal.cursor_x < TEXTBUFFER_WIDTH - 1) {
		ram->terminal.cursor_x++;
	} else {
		_cursor_newline(ram);
	}
}

static void _term_backspace(ram_t *ram) {
	
}

void term_putchar(ram_t *ram, uint8_t c, color_t bg_color, color_t fg_color) {
	if (c == '\n') {
		_cursor_newline(ram);
		return;
	}
	if (c == '\b') {
		_term_backspace(ram);
		return;
	}

	ram->textbuffer.data[_get_cursor_index(&ram->terminal)] = (char_t){
		.c = c,
		.bg_color = bg_color,
		.fg_color = fg_color,
	};

	_increment_cursor(ram);
}

void term_print(ram_t *ram, string_t string, color_t bg_color, color_t fg_color) {
	for (size_t i = 0; i < string.len; i++) {
		term_putchar(ram, string.data[i], bg_color, fg_color);
	}
}