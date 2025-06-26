#include "code.h"

#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#include "../backend/gfx.h"
#include "../backend/input.h"
#include "../backend/gui.h"
#include "../backend/text_file.h"
#include "menu.h"

// TODO: These 3 should be configurable thus stored in RAM
static const int _cursor_blink_speed = 45;

// TODO: use ram config thing
// static const int CODE_EDITOR_FONT_INDEX = 2;
static const int _scroll_speed = 3;

static int _scroll_amount = 0;
static int _lines_on_screen = 0;
static int _cursor_timer = _cursor_blink_speed;

typedef struct layout {
	rect_t code_rect;
} layout_t;

static const layout_t _layout = {
	.code_rect = {{68, 24, 568, 452}},
};

static void _move_cursor_to_mouse(ram_t *ram, file_t *code) {
	font_t *font = &ram->fonts[CODE_EDITOR_FONT_INDEX];

	point_t mouse_pos = input_get_mouse_pos();

	int corrected_x = mouse_pos.x - (_layout.code_rect.x + 5 * (font->widths[0] + font->horizontal_space));
	int corrected_y = mouse_pos.y - _layout.code_rect.y + (_scroll_amount * (font->height + font->vertical_space));

	size_t line = corrected_y / (font->height + font->vertical_space);
	if (line < 0) {
		return;
	}
	if (line >= code->line_amount) {
		line = code->line_amount - 1;
	}

 	// int pos = gui_x_to_text_index(font, code->lines[line].text, corrected_x);
	int pos = gui_x_to_string_index(font, code->lines[line].string, corrected_x);
	if (pos < 0) {
		return;
	}
	int line_len = code->lines[line].string.len;
	if (line_len < pos) {
		pos = line_len;
	}

	code->cursor_line = line;
	code->cursor_pos = pos;
}

static void _unblink_cursor() {
	_cursor_timer = _cursor_blink_speed;
}

static int _get_real_cursor_pos(computer_t *computer) {
	// if (computer->file.cursor_line == 0) {
	// 	return 0;
	// }

	return gui_get_string_width(&computer->ram->fonts[CODE_EDITOR_FONT_INDEX], computer->file.lines[computer->file.cursor_line].string, computer->file.cursor_pos);
}

void code_editor_init(computer_t *computer) {
	// TODO: Changed workspace_rect.h to layout.code_rect.h without knowing the implications, might wanna check that later
	_lines_on_screen = _layout.code_rect.h / (computer->ram->fonts[CODE_EDITOR_FONT_INDEX].height + computer->ram->fonts[CODE_EDITOR_FONT_INDEX].vertical_space);

	file_load(&computer->file, computer->code_buffer);
}

// Handle all the character inputs
static void _handle_char_input(computer_t *computer, file_t *code) {
	// Letters
	for (int i = KEY_A; i <= KEY_Z; i++) {
		if (input_key_pressed(i)) {
			if (input_key_held(KEY_LSHIFT) || input_key_held(KEY_RSHIFT)) {
				file_insert_char_at_cursor(code, 'A' + (i - KEY_A));
			} else {
				file_insert_char_at_cursor(code, 'a' + (i - KEY_A));
			}
		}
	}

	// Number row
	if (input_key_held(KEY_LSHIFT) || input_key_held(KEY_RSHIFT)) {
		if (input_key_pressed(KEY_1))
			file_insert_char_at_cursor(code, '!');

		if (input_key_pressed(KEY_2))
			file_insert_char_at_cursor(code, '@');
		
		if (input_key_pressed(KEY_3))
			file_insert_char_at_cursor(code, '#');

		if (input_key_pressed(KEY_4))
			file_insert_char_at_cursor(code, '$');

		if (input_key_pressed(KEY_5))
			file_insert_char_at_cursor(code, '%');

		if (input_key_pressed(KEY_6))
			file_insert_char_at_cursor(code, '^');

		if (input_key_pressed(KEY_7))
			file_insert_char_at_cursor(code, '&');

		if (input_key_pressed(KEY_8))
			file_insert_char_at_cursor(code, '*');

		if (input_key_pressed(KEY_9))
			file_insert_char_at_cursor(code, '(');

		if (input_key_pressed(KEY_0))
			file_insert_char_at_cursor(code, ')');
	} else {
		for (int i = 0; i <= 9; i++) {
			if (input_key_pressed(KEY_0 + i) || input_key_pressed(KEY_NUM0 + i)) {
				file_insert_char_at_cursor(code, '0' + i);
			}
		}
	}

	// Other characters
	if (input_key_held(KEY_LSHIFT) || input_key_held(KEY_RSHIFT)) {
		if (input_key_pressed(KEY_MINUS))
			file_insert_char_at_cursor(code, '_');

		if (input_key_pressed(KEY_EQUALS))
			file_insert_char_at_cursor(code, '+');

		if (input_key_pressed(KEY_LEFTBRACKET))
			file_insert_char_at_cursor(code, '{');

		if (input_key_pressed(KEY_RIGHTBRACKET))
			file_insert_char_at_cursor(code, '}');

		if (input_key_pressed(KEY_BACKSLASH))
			file_insert_char_at_cursor(code, '|');

		if (input_key_pressed(KEY_SEMICOLON))
			file_insert_char_at_cursor(code, ':');

		if (input_key_pressed(KEY_APOSTROPHE))
			file_insert_char_at_cursor(code, '\"');

		if (input_key_pressed(KEY_COMMA))
			file_insert_char_at_cursor(code, '<');

		if (input_key_pressed(KEY_PERIOD))
			file_insert_char_at_cursor(code, '>');

		if (input_key_pressed(KEY_SLASH))
			file_insert_char_at_cursor(code, '?');

		if (input_key_pressed(KEY_GRAVE))
			file_insert_char_at_cursor(code, '~');

	} else {
		if (input_key_pressed(KEY_MINUS) || input_key_pressed(KEY_NUMMINUS))
			file_insert_char_at_cursor(code, '-');

		if (input_key_pressed(KEY_EQUALS))
			file_insert_char_at_cursor(code, '=');

		if (input_key_pressed(KEY_LEFTBRACKET))
			file_insert_char_at_cursor(code, '[');

		if (input_key_pressed(KEY_RIGHTBRACKET))
			file_insert_char_at_cursor(code, ']');

		if (input_key_pressed(KEY_BACKSLASH))
			file_insert_char_at_cursor(code, '\\');

		if (input_key_pressed(KEY_SEMICOLON))
			file_insert_char_at_cursor(code, ';');

		if (input_key_pressed(KEY_APOSTROPHE))
			file_insert_char_at_cursor(code, '\'');

		if (input_key_pressed(KEY_COMMA))
			file_insert_char_at_cursor(code, ',');

		if (input_key_pressed(KEY_PERIOD) || input_key_pressed(KEY_NUMPERIOD))
			file_insert_char_at_cursor(code, '.');

		if (input_key_pressed(KEY_SLASH) || input_key_pressed(KEY_NUMDIVIDE))
			file_insert_char_at_cursor(code, '/');

		if (input_key_pressed(KEY_GRAVE))
			file_insert_char_at_cursor(code, '`');

		}
	
	// Some numpad stuff
	if (input_key_pressed(KEY_NUMMULTIPLY))
		file_insert_char_at_cursor(code, '*');

	if (input_key_pressed(KEY_NUMPLUS))
		file_insert_char_at_cursor(code, '+');

	if (input_key_pressed(KEY_TAB)) {
		file_insert_char_at_cursor(code, '\t');
	}
}

void code_editor_update(computer_t *computer) {
	file_t *file = &computer->file;

	// Cursor movement
	if (input_key_pressed(KEY_LEFT)) {
		if (input_key_held(KEY_LCTRL) || input_key_held(KEY_RCTRL)) {
			file_move_cursor_to_prev_word(file);
		} else {
			file_move_cursor_left(file);
		}

		_unblink_cursor();
	}

	// TODO: move this to backend
	if (input_key_pressed(KEY_RIGHT)) {
		if (input_key_held(KEY_LCTRL) || input_key_held(KEY_RCTRL)) {
			file_move_cursor_to_next_word(file);
		} else {
			file_move_cursor_right(file);
		}

		_unblink_cursor();
	}

	if (input_key_pressed(KEY_UP)) {
		file_move_cursor_up(file);
		_unblink_cursor();
	}

	if (input_key_pressed(KEY_DOWN)) {
		file_move_cursor_down(file);
		_unblink_cursor();
	}

	// TODO: page up, page down, home, end

	// Handle space
	if (input_key_pressed(KEY_SPACE)) {
		file_insert_char_at_cursor(file, ' ');
	}

	if (input_key_pressed(KEY_BACKSPACE)) {
		file_remove_char_at_cursor(file);
	}
	
	// Handle return
	if (input_key_pressed(KEY_RETURN) || input_key_pressed(KEY_NUMENTER)) {
		// file_split_line_down(file, file->cursor_line, file->cursor_pos, string_get_indent_level(file->lines[file->cursor_line].text));
		file_split_line_down(file, file->cursor_line, file->cursor_pos, 0);
		file->cursor_line++;
		file->cursor_pos = 0;
	}

	_handle_char_input(computer, file);

	// Mouse
	if (input_mouse_button_pressed(MOUSE_BUTTON_LEFT)) {
		_move_cursor_to_mouse(computer->ram, file);
		_unblink_cursor();
	}

	// Scrolling
	if (input_mouse_scrolled(SCROLL_DIR_DOWN)) {
		_scroll_amount += _scroll_speed;
		if (_scroll_amount >= file->line_amount) {
			_scroll_amount = file->line_amount - 1;
		}
	} else if (input_mouse_scrolled(SCROLL_DIR_UP)) {
		_scroll_amount -= _scroll_speed;
		if (_scroll_amount < 0) {
			_scroll_amount = 0;
		}
	}
}

void code_editor_draw(computer_t *computer) {
	font_t *font = &computer->ram->fonts[CODE_EDITOR_FONT_INDEX];
	surface_t fb_surf = FB_SURF(computer->ram->framebuffer.data);

	// gui_inset_frame(computer->ram, _layout.code_rect);
	gfx_draw_filled_rect(fb_surf, _layout.code_rect, computer->ram->code_editor_config.background_color);

	// TODO: replace with temp alloc (maybe)
	char line_number_buffer[8];

	// TODO: fix font so I can refactor this hardcoded mess
	for (int i = 0; i < _lines_on_screen; i++) {
		if (i + _scroll_amount >= computer->file.line_amount) {
			break;
		}

		// Commented out: version with leading zeroes
		// sprintf(line_number_buffer, "%04d", i + _scroll_amount + 1);
		sprintf(line_number_buffer, "% 4d", i + _scroll_amount + 1);

		// Line number
		gui_draw_text(computer->ram, CODE_EDITOR_FONT_INDEX, line_number_buffer, POINT(_layout.code_rect.x + 2, _layout.code_rect.y + 2 + (i * (font->height + font->vertical_space))), 8);
		
		// Line itself using tokens for syntax highlighting
		line_t *current_line = &computer->file.lines[i + _scroll_amount];
		size_t current_x = _layout.code_rect.x + 2 + 5 * (font->widths[0] + font->horizontal_space);
		// printf("font width: %d\n", font->widths[0]);

		for (size_t j = 0; j < current_line->tokens_len; j++) {
			color_t color = computer->ram->code_editor_config.token_colors[current_line->tokens[j].type];
			gui_draw_string(computer->ram, CODE_EDITOR_FONT_INDEX, current_line->tokens[j].string, POINT(current_x, _layout.code_rect.y + 2 + (i * (font->height + font->vertical_space))), color);
			current_x += gui_get_string_width(font, current_line->tokens[j].string, current_line->tokens[j].string.len);
		}
	}

	// Draw cursor
	if (_cursor_timer >= _cursor_blink_speed / 2) {
		int cursor_x = _layout.code_rect.x + 2 + 5 * (font->widths[0] + font->horizontal_space) + _get_real_cursor_pos(computer);
		int cursor_y = _layout.code_rect.y + 2 + computer->file.cursor_line * (font->height + font->vertical_space) - _scroll_amount * (font->height + font->vertical_space);
		gfx_draw_line(fb_surf, POINT(cursor_x, cursor_y), POINT(cursor_x, cursor_y + font->height), 3);
	}
	
	// Update cursor blink
	_cursor_timer--;
	if (_cursor_timer == 0) {
		_unblink_cursor();
	}
}
