#include "code.h"

#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#include "../api/api.h"
#include "../util/util.h"
#include "../backend/backend.h"
#include "../backend/input.h"
#include "menu.h"

static rect_t code_rect = {
	.x = 4,
	.y = 22,
	.w = SCREEN_WIDTH - 8,
	.h = SCREEN_HEIGHT - 20 - 6,
};

static const int font_index = 2;
static int scroll_amount = 0;

static int lines_on_screen = 0;

static char sample_string[] =	"local x = 0\n"
								"local y = 0\n"
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
								"	local color = (1 + math.sin(ticks() / 500)) * 0.5 * 256\n"
								"	cls(color)\n"
								"\n"
								"	for i=1,240 do\n"
								"		circ(320, 240, i, 256-i)\n"
								"	end\n"
								// "\n"
								// "	local radius1 = (1 + math.sin(ticks() / 10)) * 0.5 * 240\n"
								// "	local radius2 = (1 + math.cos(ticks() / 10)) * 0.5 * 240\n"
								// "	circ(320, 240, radius1, 256 - radius1)\n"
								// "	circ(320, 240, radius1 - 1, 256 - radius1)\n"
								// "	circ(320, 240, radius1 - 2, 256 - radius1)\n"
								
								// "	circ(320, 240, radius2, 256 - radius2)\n"
								// "	circ(320, 240, radius2 - 1, 256 - radius2)\n"
								// "	circ(320, 240, radius2 - 2, 256 - radius2)\n"
								"\n"
								"	spr(768, x, y, 8, 8)\n"
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
static void line_append(code_t *code, const char *text, size_t len) {
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
static void string_to_code(code_t *code, char *text) {
	char *last_line_start = text;
	size_t pos_since_last_line_start = 0;
	
	for (uint64_t i = 0; i < strlen(text) + 1; i++) {
		if (text[i] == '\0') {
			break;
		}

		if (text[i] == '\n') {
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
static size_t code_get_len(code_t *code) {
	size_t len = 0;

	for (int i = 0; i < code->line_amount; i++) {
		len += strlen(code->lines[i].text);
	}

	return len;
}

// Split the line at the given position in 2
// a new line will be created with anything on the current line after the given pos
static void split_line_at(code_t *code, int line, int pos, int indent_level) {
	// Realloc lines (not for now)
	code->lines = realloc(code->lines, ((size_t)code->line_amount + 1ULL) * sizeof(line_t));

	// Move the lines
	memmove(&code->lines[line + 1], &code->lines[line], ((size_t)code->line_amount - (size_t)line) * sizeof(line_t));
	code->line_amount++;

	char *after_cursor = &code->lines[line].text[pos];
	int after_cursor_len = strlen(after_cursor);

	code->lines[line + 1].text = malloc(((size_t)after_cursor_len + 1ULL) * sizeof(char));
	if (code->lines[line + 1].text == NULL) {
		printf("Couldn't allocate memory for new line\n");
		exit(1);
	}

	memcpy(code->lines[line + 1].text, after_cursor, (size_t)after_cursor_len + 1ULL);
	code->lines[line + 1].text[after_cursor_len] = '\0';

	code->lines[line].text[pos] = '\0';
	code->lines[line].text = realloc(code->lines[line].text, strlen(code->lines[line].text) + 1ULL);

	// TODO: Make this work
	// if (strlen(code->lines[line + 1].text) == 0) {
	// 	for (int i = 0; i < indent_level; i++) {
	// 		code->lines[line + 1].text[i] = '\t';
	// 	}
	// }
}

// Merge the given line with the line above it
static int merge_line(code_t *code, int line) {
	size_t old_line_len = strlen(code->lines[line - 1].text);
	size_t new_line_len = old_line_len + strlen(code->lines[line].text);

	code->lines[line - 1].text = realloc(code->lines[line - 1].text, ((size_t)new_line_len + 1ULL) * sizeof(char));
	strcat(code->lines[line - 1].text, code->lines[line].text);

	free(code->lines[line].text);
	code->lines[line].text = NULL;

	memmove(&code->lines[line], &code->lines[line + 1], ((size_t)code->line_amount - (size_t)line - 1ULL) * sizeof(line_t));
	
	code->line_amount--;

	return old_line_len;
}

// Insert a char at a position
static void insert_char_at(code_t *code, int line, int pos, char c) {
	size_t len = strlen(code->lines[line].text);

	// + 2, 1 for null terminator and 1 for the new character
	code->lines[line].text = realloc(code->lines[line].text, (size_t)len + 2ULL);
	memmove(&code->lines[line].text[pos] + 1, &code->lines[line].text[pos], strlen(&code->lines[line].text[pos]) + 1ULL);

	code->lines[line].text[pos] = c;
}

// Remove a char at a position
static void remove_char_at(code_t *code, int line, int pos) {
	if (pos == 0) {
		return;
	}

	size_t len = strlen(code->lines[line].text);

	memmove(&code->lines[line].text[pos] - 1, &code->lines[line].text[pos], strlen(&code->lines[line].text[pos]) + 1ULL);
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

static void move_cursor_to_mouse(ram_t *ram, code_t *code) {
	font_t *font = &ram->fonts[font_index];

	int x, y;
	get_mouse_pos(&x, &y);

	int corrected_x = x - (code_rect.x + 5 * (font->width + font->horizontal_space));
	int corrected_y = y - code_rect.y + (scroll_amount * (font->height + font->vertical_space));

	int line = corrected_y / (font->height + font->vertical_space);
	if (line < 0) {
		return;
	}
	if (line >= code->line_amount) {
		line = code->line_amount - 1;
	}

	int pos = x_to_text_index(font, code->lines[line].text, corrected_x);
	if (pos < 0) {
		return;
	}
	int line_len = strlen(code->lines[line].text);
	if (line_len < pos) {
		pos = line_len;
	}

	code->cursor_line = line;
	code->cursor_pos = pos;
}

static int get_indent_level(char text[]) {
	int indent = 0;

	for (size_t i = 0; i < strlen(text); i++) {
		if (text[i] == '\t') {
			indent++;
		} else {
			break;
		}
	}

	return indent;
}

void code_editor_init(computer_t *computer) {
	// uint64_t lines_amount = string_get_lines_amount(sample_string);
	uint64_t lines_amount = string_get_lines_amount(computer->ram->code_buffer);

	lines_on_screen = workspace_rect.h / (computer->ram->fonts[font_index].height + computer->ram->fonts[font_index].horizontal_space);

	computer->code.lines = malloc(lines_amount * sizeof(line_t));
	if (computer->code.lines == NULL) {
		printf("Couldn't allocate memory for code\n");
		exit(1);
	}

	// string_to_code(&computer->code, sample_string);
	string_to_code(&computer->code, computer->ram->code_buffer);
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
		split_line_at(code, code->cursor_line, code->cursor_pos, get_indent_level(code->lines[code->cursor_line].text));
		code->cursor_line++;
		code->cursor_pos = 0;
	}

	handle_char_input(computer, code);

	// Mouse
	if (api_mouse_btnp(computer, MOUSE_BUTTON_LEFT)) {
		move_cursor_to_mouse(computer->ram, &computer->code);
	}

	// Scrolling
	if (api_mouse_scrolled(computer, SCROLL_DOWN)) {
		scroll_amount += 3;
		if (scroll_amount >= code->line_amount) {
			scroll_amount = code->line_amount - 1;
		}
	} else if (api_mouse_scrolled(computer, SCROLL_UP)) {
		scroll_amount -= 3;
		if (scroll_amount < 0) {
			scroll_amount = 0;
		}
	}
}

static int get_real_cursor_pos(computer_t *computer) {
	return get_text_width(&computer->ram->fonts[font_index], computer->code.lines[computer->code.cursor_line].text, computer->code.cursor_pos);
}

void code_editor_draw(computer_t *computer) {
	font_t *font = &computer->ram->fonts[font_index];

	draw_in_frame(computer, code_rect);
	// api_rectf(computer, code_rect.x, code_rect.y, code_rect.w, code_rect.h, 15);
	api_rectf(computer, code_rect.x, code_rect.y, code_rect.w, code_rect.h, 15);

	// TODO: replace with temp alloc
	char line_number_buffer[8];

	// TODO: fix font so I can refactor this hardcoded mess
	for (int i = 0; i < lines_on_screen; i++) {
		if (i + scroll_amount >= computer->code.line_amount) {
			break;
		}

		sprintf(line_number_buffer, "% 4d", i + scroll_amount + 1);
		api_text(computer, font_index, line_number_buffer, code_rect.x + 2, code_rect.y + 2 + (i * (font->height + font->vertical_space)), 8);
		api_text(computer, font_index, computer->code.lines[i + scroll_amount].text, code_rect.x + 2 + 5 * (font->width + font->horizontal_space), code_rect.y + 2 + (i * (font->height + font->vertical_space)), 0);
	}

	// Draw cursor
	if (computer->ticks % 40 < 20) {
		int cursor_x = code_rect.x + 2 + 5 * (font->width + font->horizontal_space) + get_real_cursor_pos(computer);
		int cursor_y = code_rect.y + 2 + computer->code.cursor_line * (font->height + font->vertical_space) - scroll_amount * (font->height + font->vertical_space);
		api_line(computer, cursor_x, cursor_y, cursor_x, cursor_y + font->height, 3);
	}
}
