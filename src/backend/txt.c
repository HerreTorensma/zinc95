#include "txt.h"

#include <string.h>
#include "input.h"

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

	// TODO: Cursor
	for (int y = computer->ram->terminal.cursor_y * TEXTBUFFER_CHAR_HEIGHT; y < computer->ram->terminal.cursor_y * TEXTBUFFER_CHAR_HEIGHT + TEXTBUFFER_CHAR_HEIGHT; y++) {
		for (int x = computer->ram->terminal.cursor_x * TEXTBUFFER_CHAR_WIDTH; x < computer->ram->terminal.cursor_x * TEXTBUFFER_CHAR_WIDTH + TEXTBUFFER_CHAR_WIDTH; x++) {
			computer->rgb_framebuffer[y * SCREEN_WIDTH + x] = (rgb_color_t){255, 255, 255};
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
	ram->terminal.cursor_x = 0;
	ram->terminal.cursor_y = 0;
}

static size_t _get_cursor_index(terminal_t *terminal) {
	return (size_t)terminal->cursor_y * (size_t)TEXTBUFFER_WIDTH + (size_t)terminal->cursor_x;
}

static void _scroll_if_needed(ram_t *ram) {
	if (ram->terminal.cursor_y >= TEXTBUFFER_HEIGHT) {
		txt_shift_lines_up(ram);
		ram->terminal.cursor_y = TEXTBUFFER_HEIGHT - 1;
	}
}

static void _newline(ram_t *ram) {
	ram->terminal.cursor_x = 0;
	ram->terminal.cursor_y++;

	_scroll_if_needed(ram);
}

static void _increment_cursor(ram_t *ram) {
	if (ram->terminal.cursor_x < TEXTBUFFER_WIDTH - 1) {
		ram->terminal.cursor_x++;
	} else {
		_newline(ram);
	}
}

static void _term_backspace(ram_t *ram) {
	if (ram->terminal.cursor_x > 0) {
		ram->terminal.cursor_x--;
	} else if (ram->terminal.cursor_y > 0) {
		ram->terminal.cursor_y--;
		ram->terminal.cursor_x = TEXTBUFFER_WIDTH - 1;
	}

	size_t index = _get_cursor_index(&ram->terminal);
	ram->textbuffer.data[index] = (char_t){
		.c = '\0',
		.bg_color = 0,
		.fg_color = 0,
	};
}

void term_putchar(ram_t *ram, uint8_t c, color_t bg_color, color_t fg_color) {
	if (c == '\n') {
		_newline(ram);
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

void term_printc(ram_t *ram, string_t string, color_t bg_color, color_t fg_color) {
	for (size_t i = 0; i < string.len; i++) {
		term_putchar(ram, string.data[i], bg_color, fg_color);
	}
}

void term_print(ram_t *ram, string_t string) {
	term_printc(ram, string, COLOR_BLACK, COLOR_WHITE);
}

char term_getchar() {
	return input_get_as_char();
}

// TODO: use this instead of just the shell_update function
string_t term_input(string_t string) {

}

static void _print_help(ram_t *ram) {
	term_print(ram, STR("load <filename> load a file\n"));
	term_print(ram, STR("save <filename> save a file\n"));
	term_print(ram, STR("run             run the currently loaded game\n"));
	term_print(ram, STR("resume          resume the currently running game\n"));
	term_print(ram, STR("cd <dirname>    enter a directory\n"));
	term_print(ram, STR("cd ..           enter the parent directory\n"));
	term_print(ram, STR("ls              list the files in the current directory\n"));
	term_print(ram, STR("mkdir <dirname> create a new directory\n"));
	term_print(ram, STR("clear           clear the screen\n"));
	term_putchar(ram, '\n', 0, 0);
	term_print(ram, STR("Press F11 to toggle fullscreen\n"));
	term_putchar(ram, '\n', 0, 0);
}

static void _execute_command(ram_t *ram, string_t input) {
	// term_print(ram, STR("Executing\n"), 0, 15);
	if (string_eq(input, STR("help"))) {
		_print_help(ram);
	} else if (string_eq(input, STR("clear"))) {
		txt_clear(ram);
	} else {
		term_printc(ram, STR("Syntax error\n"), COLOR_BLACK, COLOR_RED);
	}
}

void shell_new_command(ram_t *ram) {
	ram->shell.line_len = 0;
	term_printc(ram, STR(">"), 0, 7);
}

static void _print_intro(ram_t *ram) {
	term_printc(ram, STR("Zinc"), 0, 7);
	term_printc(ram, STR("95\n"), 0, 12);
	term_printc(ram, STR("Enter help for help\n"), 0, 15);
	term_putchar(ram, '\n', 0, 0);
}

void shell_init(ram_t *ram) {
	_print_intro(ram);
	shell_new_command(ram);
}

void shell_update(ram_t *ram) {
	char c = term_getchar();
	if (c == '\0') {
		return;
	}
	
	if (c == '\n') {
		// We have the command
		term_putchar(ram, c, 0, 15);
		string_t input = (string_t) {
			.data = ram->shell.line_buffer,
			.len = ram->shell.line_len,
		};

		_execute_command(ram, input);
		shell_new_command(ram);
	} else if (c == '\b') {
		// Backspace
		if (ram->shell.line_len > 0) {
			ram->shell.line_len--;
			_term_backspace(ram);
		}
	} else if (ram->shell.line_len < 80 - 1) {
		ram->shell.line_buffer[ram->shell.line_len] = c;
		ram->shell.line_len++;
		term_putchar(ram, c, 0, 15);
	}
}