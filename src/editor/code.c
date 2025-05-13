#include "code.h"

#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#include "../api/api.h"
#include "../backend/input.h"
#include "../backend/gui.h"
#include "../backend/text_file.h"
#include "menu.h"

static const int font_index = 2;
static int scroll_amount = 0;

static int lines_on_screen = 0;

#define CURSOR_BLINK_SPEED 30
static int cursor_timer = CURSOR_BLINK_SPEED;

typedef struct layout {
	rect_t code_rect;
} layout_t;

static layout_t layout = {0};

static void move_cursor_to_mouse(ram_t *ram, file_t *code) {
	font_t *font = &ram->fonts[font_index];

	// int x, y;
	// get_mouse_pos(&x, &y);
	point_t mouse_pos = input_get_mouse_pos();

	int corrected_x = mouse_pos.x - (layout.code_rect.x + 5 * (font->width + font->horizontal_space));
	int corrected_y = mouse_pos.y - layout.code_rect.y + (scroll_amount * (font->height + font->vertical_space));

	int line = corrected_y / (font->height + font->vertical_space);
	if (line < 0) {
		return;
	}
	if (line >= code->line_amount) {
		line = code->line_amount - 1;
	}

	int pos = gui_x_to_text_index(font, code->lines[line].text, corrected_x);
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

static void unblink_cursor() {
	cursor_timer = CURSOR_BLINK_SPEED;
}

static int get_real_cursor_pos(computer_t *computer) {
	return gui_get_text_width(&computer->ram->fonts[font_index], computer->file.lines[computer->file.cursor_line].text, computer->file.cursor_pos);
}

void code_editor_init(computer_t *computer) {
	layout = (layout_t){
		.code_rect = RECT(68, 24, 568, 452),
	};

	uint64_t lines_amount = string_get_lines_amount(computer->code_buffer);

	// TODO: Changed workspace_rect.h to layout.code_rect.h without knowing the implications, might wanna check that later
	lines_on_screen = layout.code_rect.h / (computer->ram->fonts[font_index].height + computer->ram->fonts[font_index].horizontal_space);

	computer->file.lines = malloc(lines_amount * sizeof(line_t));
	if (computer->file.lines == NULL) {
		printf("Couldn't allocate memory for code\n");
		exit(1);
	}

	// string_to_code(&computer->code, sample_string);
	string_to_code(&computer->file, computer->code_buffer);
}

// Handle all the character inputs
static void handle_char_input(computer_t *computer, file_t *code) {
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

void code_editor_update(computer_t *computer) {
	file_t *code = &computer->file;
	
	// Cursor movement
	if (api_keyp(computer, KEY_LEFT)) {
		if (api_key(computer, KEY_LCTRL) || api_key(computer, KEY_RCTRL)) {
			for (int i = code->cursor_pos - 1; i >= 0; i--) {
				if (code->lines[code->cursor_line].text[i] == ' ' || code->lines[code->cursor_line].text[i] == '.' || i == 0) {
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

		unblink_cursor();
	}

	// TODO: move this to backend
	if (api_keyp(computer, KEY_RIGHT)) {
		if (api_key(computer, KEY_LCTRL) || api_key(computer, KEY_RCTRL)) {
			int len = strlen(code->lines[code->cursor_line].text);
			
			for (int i = code->cursor_pos + 1; i < len + 1; i++) {
				if (code->lines[code->cursor_line].text[i] == ' ' || code->lines[code->cursor_line].text[i] == '.' || i == len) {
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

		unblink_cursor();
	}

	if (api_keyp(computer, KEY_UP)) {
		if (code->cursor_line > 0) {
			code->cursor_line--;

			int len = strlen(code->lines[code->cursor_line].text);
			if (code->cursor_pos > len) {
				code->cursor_pos = len;
			}
		}

		unblink_cursor();
	}

	if (api_keyp(computer, KEY_DOWN)) {
		if (code->cursor_line < code->line_amount - 1) {
			code->cursor_line++;
		}

		int len = strlen(code->lines[code->cursor_line].text);
		if (code->cursor_pos > len) {
			code->cursor_pos = len;
		}

		unblink_cursor();
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
		move_cursor_to_mouse(computer->ram, &computer->file);
		unblink_cursor();
	}

	// Scrolling
	if (api_mouse_scrolled(computer, SCROLL_DIR_DOWN)) {
		scroll_amount += 3;
		if (scroll_amount >= code->line_amount) {
			scroll_amount = code->line_amount - 1;
		}
	} else if (api_mouse_scrolled(computer, SCROLL_DIR_UP)) {
		scroll_amount -= 3;
		if (scroll_amount < 0) {
			scroll_amount = 0;
		}
	}
}

void code_editor_draw(computer_t *computer) {
	font_t *font = &computer->ram->fonts[font_index];

	gui_inset_frame(computer->ram, layout.code_rect);
	// api_rectf(computer, code_rect.x, code_rect.y, code_rect.w, code_rect.h, 15);
	api_rectf(computer, layout.code_rect.x, layout.code_rect.y, layout.code_rect.w, layout.code_rect.h, 15);

	// TODO: replace with temp alloc
	char line_number_buffer[8];

	// TODO: fix font so I can refactor this hardcoded mess
	for (int i = 0; i < lines_on_screen; i++) {
		if (i + scroll_amount >= computer->file.line_amount) {
			break;
		}

		// Commented out: version with leading zeroes
		// sprintf(line_number_buffer, "%04d", i + scroll_amount + 1);
		
		sprintf(line_number_buffer, "% 4d", i + scroll_amount + 1);
		api_text(computer, font_index, line_number_buffer, layout.code_rect.x + 2, layout.code_rect.y + 2 + (i * (font->height + font->vertical_space)), 8);
		api_text(computer, font_index, computer->file.lines[i + scroll_amount].text, layout.code_rect.x + 2 + 5 * (font->width + font->horizontal_space), layout.code_rect.y + 2 + (i * (font->height + font->vertical_space)), 0);
	}

	// Draw cursor
	// if (computer->ticks % 40 < 20) {
	if (cursor_timer >= CURSOR_BLINK_SPEED / 2) {
		int cursor_x = layout.code_rect.x + 2 + 5 * (font->width + font->horizontal_space) + get_real_cursor_pos(computer);
		int cursor_y = layout.code_rect.y + 2 + computer->file.cursor_line * (font->height + font->vertical_space) - scroll_amount * (font->height + font->vertical_space);
		api_line(computer, cursor_x, cursor_y, cursor_x, cursor_y + font->height, 3);
	}
	
	// Update cursor blink
	cursor_timer--;
	if (cursor_timer == 0) {
		unblink_cursor();
	}
}
