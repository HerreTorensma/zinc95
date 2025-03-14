#include "code.h"

#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#include "../api/api.h"
#include "../util/util.h"
#include "../backend/input.h"

static rect_t code_rect = {
	.x = 4,
	.y = 22,
	.w = SCREEN_WIDTH - 8,
	.h = SCREEN_HEIGHT - 20 - 6,
};

static const int font_index = 1;

static char sample_string[] =	"local x = 0\n"
								"local y = 50\n"
								"\n"
								"function _init()\n"
								"	print(\"Called the init function\")\n"
								"end\n"
								"\n"
								"function _update()\n"
								"	x = x + 1\n"
								"	y = y + 1\n"
								"end\n"
								"\n"
								"function _draw()\n"
								"	cls(0)\n"
								"	spr(0, 0, x, y, 1, 1)\n"
								"end\0";

// Get the amount of lines in a string, used for loading
static uint64_t string_get_lines_amount(const char *text) {
	uint64_t amount = 0;

	for (uint64_t i = 0; i < strlen(text); i++) {
		if (text[i] == '\n' || text[i] == '\0') {
			amount++;
		}
	}

	amount++;

	return amount;
}

// Append a new line, used for loading a string before editing
static void line_append(code_t *code, const char *text, int len) {
	code->lines[code->line_amount].text = malloc((len + 1) * sizeof(char));
	if (code->lines[code->line_amount].text == NULL) {
		printf("Couldn't allocate memory for new line\n");
		exit(1);
	}

	memcpy(code->lines[code->line_amount].text, text, len);

	code->lines[code->line_amount].text[len] = '\0';
	code->line_amount++;
}

// Load a string into the code_t datastructure
static void string_to_code(code_t *code, const char *text) {
	char *last_line_start = text;
	int pos_since_last_line_start = 0;
	
	for (uint64_t i = 0; i < strlen(text) + 1; i++) {
		if (text[i] == '\n' || text[i] == '\0') {
			if (last_line_start != text) {
				last_line_start++;
			}
			line_append(code, last_line_start, pos_since_last_line_start);

			pos_since_last_line_start = 0;
			last_line_start = &text[i];
			
			continue;
		}

		pos_since_last_line_start++;
	}
}

// Get size of code string
static uint64_t code_get_len(code_t *code) {
	uint64_t len = 0;

	for (int i = 0; i < code->line_amount; i++) {
		len += strlen(code->lines[i].text);
	}

	return len;
}

// Convert the code_t datastructure back to a string for saving
// the function assumes that passed buffer is large enough
void code_to_string(code_t *code, char *buffer) {
	uint64_t buffer_pos = 0;

	for (int i = 0; i < code->line_amount; i++) {
		int line_len = strlen(code->lines[i].text);
		
		memcpy(&buffer[buffer_pos], code->lines[i].text, (line_len + 1) * sizeof(char));
		
		if (i < code->line_amount - 1) {
			buffer[buffer_pos + line_len] = '\n';
		} else {
			buffer[buffer_pos + line_len] = '\0';
		}
		buffer_pos += line_len + 1;
	}
}

// Split the line at the given position in 2
// a new line will be created with anything on the current line after the given pos
static void split_line_at(code_t *code, uint64_t line, uint64_t pos) {
	// Realloc lines (not for now)
	code->lines = realloc(code->lines, (code->line_amount + 1) * sizeof(line_t));

	// Move the lines
	memmove(&code->lines[line + 1], &code->lines[line], (code->line_amount - line) * sizeof(line_t));
	code->line_amount++;

	char *after_cursor = &code->lines[line].text[pos];
	int after_cursor_len = strlen(after_cursor);

	code->lines[line + 1].text = malloc((after_cursor_len + 1) * sizeof(char));
	if (code->lines[line + 1].text == NULL) {
		printf("Couldn't allocate memory for new line\n");
		exit(1);
	}

	memcpy(code->lines[line + 1].text, after_cursor, after_cursor_len + 1);
	code->lines[line + 1].text[after_cursor_len] = '\0';

	code->lines[line].text[pos] = '\0';
	code->lines[line].text = realloc(code->lines[line].text, strlen(code->lines[line].text) + 1);
}

// Merge the given line with the line above it
static int merge_line(code_t *code, uint64_t line) {
	int old_line_len = strlen(code->lines[line - 1].text);
	int new_line_len = old_line_len + strlen(code->lines[line].text);

	code->lines[line - 1].text = realloc(code->lines[line - 1].text, (new_line_len + 1) * sizeof(char));
	strcat(code->lines[line - 1].text, code->lines[line].text);

	free(code->lines[line].text);
	code->lines[line].text = NULL;

	memmove(&code->lines[line], &code->lines[line + 1], (code->line_amount - line - 1) * sizeof(line_t));
	
	code->line_amount--;

	return old_line_len;
}

// Insert a char at a position
static void insert_char_at(code_t *code, uint64_t line, uint64_t pos, char c) {
	int len = strlen(code->lines[line].text);

	// + 2, 1 for null terminator and 1 for the new character
	code->lines[line].text = realloc(code->lines[line].text, len + 2);
	memmove(&code->lines[line].text[pos] + 1, &code->lines[line].text[pos], strlen(&code->lines[line].text[pos]) + 1);

	code->lines[line].text[pos] = c;
}

// Remove a char at a position
static void remove_char_at(code_t *code, uint64_t line, uint64_t pos) {
	if (pos == 0) {
		return;
	}

	int len = strlen(code->lines[line].text);

	memmove(&code->lines[line].text[pos] - 1, &code->lines[line].text[pos], strlen(&code->lines[line].text[pos]) + 1);
	code->lines[line].text = realloc(code->lines[line].text, len);

	code->cursor_pos--;
}

// Wrapper
static inline void insert_char_at_cursor(code_t *code, char c) {
	insert_char_at(code, code->cursor_line, code->cursor_pos, c);
	code->cursor_pos++;
}

// Handle all the character inputs
static void handle_char_input(computer_t *computer, code_t *code) {
	// Letters
	for (int i = KEY_A; i <= KEY_Z; i++) {
		if (api_keyp(computer, i)) {
			if (api_key(computer, KEY_LSHIFT) || api_key(computer, KEY_RSHIFT)) {
				insert_char_at_cursor(code, 'A' + (i - KEY_A));
			} else {
				insert_char_at_cursor(code, 'a' + (i - KEY_A));
			}
		}
	}

	// Number row
	if (api_key(computer, KEY_LSHIFT) || api_key(computer, KEY_RSHIFT)) {
		if (api_keyp(computer, KEY_1))
			insert_char_at_cursor(code, '!');

		if (api_keyp(computer, KEY_2))
			insert_char_at_cursor(code, '@');
		
		if (api_keyp(computer, KEY_3))
			insert_char_at_cursor(code, '#');

		if (api_keyp(computer, KEY_4))
			insert_char_at_cursor(code, '$');

		if (api_keyp(computer, KEY_5))
			insert_char_at_cursor(code, '%');

		if (api_keyp(computer, KEY_6))
			insert_char_at_cursor(code, '^');

		if (api_keyp(computer, KEY_7))
			insert_char_at_cursor(code, '&');

		if (api_keyp(computer, KEY_8))
			insert_char_at_cursor(code, '*');

		if (api_keyp(computer, KEY_9))
			insert_char_at_cursor(code, '(');

		if (api_keyp(computer, KEY_0))
			insert_char_at_cursor(code, ')');
	} else {
		for (int i = 0; i <= 9; i++) {
			if (api_keyp(computer, KEY_0 + i) || api_keyp(computer, KEY_NUM0 + i)) {
				insert_char_at_cursor(code, '0' + i);
			}
		}
	}

	// Other characters
	if (api_key(computer, KEY_LSHIFT) || api_key(computer, KEY_RSHIFT)) {
		if (api_keyp(computer, KEY_MINUS))
			insert_char_at_cursor(code, '_');

		if (api_keyp(computer, KEY_EQUALS))
			insert_char_at_cursor(code, '+');

		if (api_keyp(computer, KEY_LEFTBRACKET))
			insert_char_at_cursor(code, '{');

		if (api_keyp(computer, KEY_RIGHTBRACKET))
			insert_char_at_cursor(code, '}');

		if (api_keyp(computer, KEY_BACKSLASH))
			insert_char_at_cursor(code, '|');

		if (api_keyp(computer, KEY_SEMICOLON))
			insert_char_at_cursor(code, ':');

		if (api_keyp(computer, KEY_APOSTROPHE))
			insert_char_at_cursor(code, '\"');

		if (api_keyp(computer, KEY_COMMA))
			insert_char_at_cursor(code, '<');

		if (api_keyp(computer, KEY_PERIOD))
			insert_char_at_cursor(code, '>');

		if (api_keyp(computer, KEY_SLASH))
			insert_char_at_cursor(code, '?');

		if (api_keyp(computer, KEY_GRAVE))
			insert_char_at_cursor(code, '~');

	} else {
		if (api_keyp(computer, KEY_MINUS) || api_keyp(computer, KEY_NUMMINUS))
			insert_char_at_cursor(code, '-');

		if (api_keyp(computer, KEY_EQUALS))
			insert_char_at_cursor(code, '=');

		if (api_keyp(computer, KEY_LEFTBRACKET))
			insert_char_at_cursor(code, '[');

		if (api_keyp(computer, KEY_RIGHTBRACKET))
			insert_char_at_cursor(code, ']');

		if (api_keyp(computer, KEY_BACKSLASH))
			insert_char_at_cursor(code, '\\');

		if (api_keyp(computer, KEY_SEMICOLON))
			insert_char_at_cursor(code, ';');

		if (api_keyp(computer, KEY_APOSTROPHE))
			insert_char_at_cursor(code, '\'');

		if (api_keyp(computer, KEY_COMMA))
			insert_char_at_cursor(code, ',');

		if (api_keyp(computer, KEY_PERIOD) || api_keyp(computer, KEY_NUMPERIOD))
			insert_char_at_cursor(code, '.');

		if (api_keyp(computer, KEY_SLASH) || api_keyp(computer, KEY_NUMDIVIDE))
			insert_char_at_cursor(code, '/');

		if (api_keyp(computer, KEY_GRAVE))
			insert_char_at_cursor(code, '`');

		}
	
	// Some numpad stuff
	if (api_keyp(computer, KEY_NUMMULTIPLY))
		insert_char_at_cursor(code, '*');

	if (api_keyp(computer, KEY_NUMPLUS))
		insert_char_at_cursor(code, '+');

	if (api_keyp(computer, KEY_TAB)) {
		insert_char_at_cursor(code, '\t');
	}
}

void code_editor_init(computer_t *computer) {
	uint64_t lines_amount = string_get_lines_amount(sample_string);

	computer->code.lines = malloc(lines_amount * sizeof(line_t));
	if (computer->code.lines == NULL) {
		printf("Couldn't allocate memory for code\n");
		exit(1);
	}

	string_to_code(&computer->code, sample_string);
}

void code_editor_update(computer_t *computer) {
	code_t *code = &computer->code;
	
	// Cursor movement
	if (api_keyp(computer, KEY_LEFT)) {
		if (api_key(computer, KEY_LCTRL) || api_key(computer, KEY_RCTRL)) {
			for (int i = code->cursor_pos - 1; i >= 0; i--) {
				if (code->lines[code->cursor_line].text[i] == ' ' || i == 0) {
					code->cursor_pos = i;
					break;
				}
			}
		} else {
			if (code->cursor_pos > 0) {
				code->cursor_pos--;
			} else {
				if (code->cursor_line > 0) {
					code->cursor_line--;
					code->cursor_pos = strlen(code->lines[code->cursor_line].text);
				}
			}
		}
	}

	if (api_keyp(computer, KEY_RIGHT)) {
		if (api_key(computer, KEY_LCTRL) || api_key(computer, KEY_RCTRL)) {
			int len = strlen(code->lines[code->cursor_line].text);
			
			for (int i = code->cursor_pos + 1; i < len + 1; i++) {
				if (code->lines[code->cursor_line].text[i] == ' ' || i == len) {
					code->cursor_pos = i;
					break;
				}
			}
		} else {
			int len = strlen(code->lines[code->cursor_line].text);
			if (code->cursor_pos < len) {
				code->cursor_pos++;
			} else {
				if (code->cursor_line < code->line_amount - 1) {
					code->cursor_line++;
					code->cursor_pos = 0;
				}
			}
		}
	}

	if (api_keyp(computer, KEY_UP)) {
		if (code->cursor_line > 0) {
			code->cursor_line--;

			int len = strlen(code->lines[code->cursor_line].text);
			if (code->cursor_pos > len) {
				code->cursor_pos = len;
			}
		}
	}

	if (api_keyp(computer, KEY_DOWN)) {
		if (code->cursor_line < code->line_amount - 1) {
			code->cursor_line++;
		}

		int len = strlen(code->lines[code->cursor_line].text);
		if (code->cursor_pos > len) {
			code->cursor_pos = len;
		}
	}

	// TODO: page up, page down, home, end

	// Handle space
	if (api_keyp(computer, KEY_SPACE)) {
		insert_char_at_cursor(code, ' ');
	}

	if (api_keyp(computer, KEY_BACKSPACE)) {
		if (code->cursor_pos == 0) {
			if (code->cursor_line > 0) {
				code->cursor_pos = merge_line(code, code->cursor_line);
				code->cursor_line--;
			}
		} else {
			remove_char_at(code, code->cursor_line, code->cursor_pos);
		}
	}
	
	// Handle return
	if (api_keyp(computer, KEY_RETURN) || api_keyp(computer, KEY_NUMENTER)) {
		split_line_at(code, code->cursor_line, code->cursor_pos);
		code->cursor_line++;
		code->cursor_pos = 0;
	}

	handle_char_input(computer, code);
}

static int get_real_cursor_pos(computer_t *computer) {
	font_meta_t *font = &computer->ram->fonts[font_index];
	int pos = 0;

	for (int i = 0; i < computer->code.cursor_pos; i++) {
		if (computer->code.lines[computer->code.cursor_line].text[i] == '\t') {
			if (font->monospace) {
				pos += (font->width + font->horizontal_space) * TAB_SIZE;
			} else {
				pos += (font->widths[computer->code.lines[computer->code.cursor_line].text[' '] - VISIBLE_CHARACTERS_START] + font->horizontal_space) * TAB_SIZE;
			}

			continue;
		}

		if (font->monospace) {
			pos += font->width + font->horizontal_space;
		} else {
			pos += font->widths[computer->code.lines[computer->code.cursor_line].text[i] - VISIBLE_CHARACTERS_START] + font->horizontal_space;
		}
	}

	return pos;
}

void code_editor_draw(computer_t *computer) {
	font_meta_t *font = &computer->ram->fonts[font_index];

	draw_in_frame(computer, code_rect);
	// api_rectf(computer, code_rect.x, code_rect.y, code_rect.w, code_rect.h, 15);
	api_rectf(computer, code_rect.x, code_rect.y, code_rect.w, code_rect.h, 15);

	// TODO: replace with temp alloc
	char line_number_buffer[8];

	// TODO: fix font so I can refactor this hardcoded mess
	for (uint64_t i = 0; i < computer->code.line_amount; i++) {
		sprintf(line_number_buffer, "% 4lld", i + 1);
		// api_text(computer, 0, line_number_buffer, code_rect.x + 2, code_rect.y + 2 + i * 10, 8);
		// api_text(computer, 0, computer->code.lines[i].text, code_rect.x + 2 + 5 * 6, code_rect.y + 2 + i * 10, 0);

		api_text(computer, font_index, line_number_buffer, code_rect.x + 2, code_rect.y + 2 + (i * (font->height + font->vertical_space)), 8);
		api_text(computer, font_index, computer->code.lines[i].text, code_rect.x + 2 + 5 * (font->width + font->horizontal_space), code_rect.y + 2 + (i * (font->height + font->vertical_space)), 0);
	}


	// Draw cursor
	if (computer->ticks % 40 < 20) {
		// api_line(computer, code_rect.x + 2 + 5 * 6 + computer->code.cursor_pos * 6, code_rect.y + 2 + computer->code.cursor_line * 10, code_rect.x + 2 + 5 * 6 + computer->code.cursor_pos * 6, code_rect.y + 2 + computer->code.cursor_line * 10 + 8, 3);
		// int cursor_x = code_rect.x + 2 + 4 * font->width + computer->code.cursor_pos * font->width;
		// int cursor_x = code_rect.x + 2 + 5 * (font->width + font->horizontal_space) + (computer->code.cursor_pos * (font->width + font->horizontal_space));
		int cursor_x = code_rect.x + 2 + 5 * (font->width + font->horizontal_space) + get_real_cursor_pos(computer);
		int cursor_y = code_rect.y + 2 + computer->code.cursor_line * (font->height + font->vertical_space);
		api_line(computer, cursor_x, cursor_y, cursor_x, cursor_y + font->height, 3);
	}
}
