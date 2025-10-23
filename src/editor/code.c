#include "code.h"

#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#include "../backend/gfx.h"
#include "../backend/input.h"
#include "../backend/gui.h"
#include "../backend/text_file.h"
#include "../backend/window.h"

#define KEY_PRESSED_OR_LONG_PRESSED(key, action) do { \
	if (input_key_pressed(key)) { \
		action; \
		_key_timers[key] = 30; \
	} \
	if (!input_key_held(key)) { \
		_key_timers[key] = 0; \
	} \
	if (_key_timers[key] == 1) { \
		action; \
		_key_timers[key] = 4; \
	} \
} while (0);

// TODO: These 3 should be configurable thus stored in RAM
static const int _cursor_blink_speed = 45;

// TODO: use ram config thing
// static const int CODE_EDITOR_FONT_INDEX = 2;
static const int _scroll_speed = 3;

static int _scroll_amount = 0;
static int _lines_on_screen = 0;
static int _cursor_timer = _cursor_blink_speed;

static int _key_timers[KEY_COUNT] = {0};

typedef struct layout {
	point_t file_buttons_pos;
	rect_t code_rect;
} layout_t;

static const layout_t _layout = {
	.file_buttons_pos = {2, 22},
	.code_rect = {{68, 24, 568, 452}},
};

static size_t _current_file_index = 0;

static void _screen_pos_to_file_pos(ram_t *ram, file_t *file, point_t screen_pos, size_t *mouse_line, size_t *mouse_pos) {
	font_t *font = &ram->fonts[CODE_EDITOR_FONT_INDEX];

	int corrected_x = screen_pos.x - (_layout.code_rect.x + 5 * (font->widths[0] + font->horizontal_space));
	int corrected_y = screen_pos.y - _layout.code_rect.y + (_scroll_amount * (font->height + font->vertical_space));

	size_t line = corrected_y / (font->height + font->vertical_space);
	if (line < 0) {
		return;
	}
	if (line >= file->line_amount) {
		line = file->line_amount - 1;
	}

 	// int pos = gui_x_to_text_index(font, file->lines[line].text, corrected_x);
	int pos = gui_x_to_string_index(font, file->lines[line].string, corrected_x);
	if (pos < 0) {
		return;
	}
	int line_len = file->lines[line].string.len;
	if (line_len < pos) {
		pos = line_len;
	}

	*mouse_line = line;
	*mouse_pos = pos;
}

static void _move_cursor_to_mouse(ram_t *ram, file_t *file) {
	_screen_pos_to_file_pos(ram, file, input_get_mouse_pos(), &file->cursor.line, &file->cursor.pos);
}

static void _unblink_cursor() {
	_cursor_timer = _cursor_blink_speed;
}

static int _get_real_cursor_pos(computer_t *computer) {
	if (computer->files[_current_file_index].line_amount > 0) {
		return gui_get_string_width(&computer->ram->fonts[CODE_EDITOR_FONT_INDEX], computer->files[_current_file_index].lines[computer->files[_current_file_index].cursor.line].string, computer->files[_current_file_index].cursor.pos);
	}

	return 0;
}

// TODO: investigate why upon switching to the code editor from another editor (sprite?) all text is selected
void code_editor_init(computer_t *computer) {
	// TODO: Changed workspace_rect.h to layout.code_rect.h without knowing the implications, might wanna check that later
	_lines_on_screen = _layout.code_rect.h / (computer->ram->fonts[CODE_EDITOR_FONT_INDEX].height + computer->ram->fonts[CODE_EDITOR_FONT_INDEX].vertical_space);

	// Not necessary anymore but I'm still keeping it commented out just in case
	// file_load(&computer->file, computer->code_buffer);
}

// Handle all the character inputs
static void _handle_char_input(computer_t *computer, file_t *file) {
	if (input_key_held(KEY_LCTRL) || input_key_held(KEY_RCTRL)) {
		return;
	}

	// Letters
	if (input_key_held(KEY_LSHIFT) || input_key_held(KEY_RSHIFT)) {
		for (int i = KEY_A; i <= KEY_Z; i++) {
			KEY_PRESSED_OR_LONG_PRESSED(i, file_remove_selection(file); file_insert_char_at_cursor(file, 'A' + (i - KEY_A)));
		}
	} else {
		for (int i = KEY_A; i <= KEY_Z; i++) {
			KEY_PRESSED_OR_LONG_PRESSED(i, file_remove_selection(file); file_insert_char_at_cursor(file, 'a' + (i - KEY_A)));
		}
	}

	// Number row
	if (input_key_held(KEY_LSHIFT) || input_key_held(KEY_RSHIFT)) {
		KEY_PRESSED_OR_LONG_PRESSED(KEY_1, file_remove_selection(file); file_insert_char_at_cursor(file, '!'));
		KEY_PRESSED_OR_LONG_PRESSED(KEY_2, file_remove_selection(file); file_insert_char_at_cursor(file, '@'));
		KEY_PRESSED_OR_LONG_PRESSED(KEY_3, file_remove_selection(file); file_insert_char_at_cursor(file, '#'));
		KEY_PRESSED_OR_LONG_PRESSED(KEY_4, file_remove_selection(file); file_insert_char_at_cursor(file, '$'));
		KEY_PRESSED_OR_LONG_PRESSED(KEY_5, file_remove_selection(file); file_insert_char_at_cursor(file, '%'));
		KEY_PRESSED_OR_LONG_PRESSED(KEY_6, file_remove_selection(file); file_insert_char_at_cursor(file, '^'));
		KEY_PRESSED_OR_LONG_PRESSED(KEY_7, file_remove_selection(file); file_insert_char_at_cursor(file, '&'));
		KEY_PRESSED_OR_LONG_PRESSED(KEY_8, file_remove_selection(file); file_insert_char_at_cursor(file, '*'));
		KEY_PRESSED_OR_LONG_PRESSED(KEY_9, file_remove_selection(file); file_insert_char_at_cursor(file, '('));
		KEY_PRESSED_OR_LONG_PRESSED(KEY_0, file_remove_selection(file); file_insert_char_at_cursor(file, ')'));
	} else {
		for (int i = 0; i <= 9; i++) {
			KEY_PRESSED_OR_LONG_PRESSED(KEY_0 + i, file_remove_selection(file); file_insert_char_at_cursor(file, '0' + i));
			KEY_PRESSED_OR_LONG_PRESSED(KEY_NUM0 + i, file_remove_selection(file); file_insert_char_at_cursor(file, '0' + i));
		}
	}

	// Other characters
	if (input_key_held(KEY_LSHIFT) || input_key_held(KEY_RSHIFT)) {
		KEY_PRESSED_OR_LONG_PRESSED(KEY_MINUS, file_remove_selection(file); file_insert_char_at_cursor(file, '_'));
		KEY_PRESSED_OR_LONG_PRESSED(KEY_EQUALS, file_remove_selection(file); file_insert_char_at_cursor(file, '+'));
		KEY_PRESSED_OR_LONG_PRESSED(KEY_LEFTBRACKET, file_remove_selection(file); file_insert_char_at_cursor(file, '{'));
		KEY_PRESSED_OR_LONG_PRESSED(KEY_RIGHTBRACKET, file_remove_selection(file); file_insert_char_at_cursor(file, '}'));
		KEY_PRESSED_OR_LONG_PRESSED(KEY_BACKSLASH, file_remove_selection(file); file_insert_char_at_cursor(file, '|'));
		KEY_PRESSED_OR_LONG_PRESSED(KEY_SEMICOLON, file_remove_selection(file); file_insert_char_at_cursor(file, ':'));
		KEY_PRESSED_OR_LONG_PRESSED(KEY_APOSTROPHE, file_remove_selection(file); file_insert_char_at_cursor(file, '\"'));
		KEY_PRESSED_OR_LONG_PRESSED(KEY_COMMA, file_remove_selection(file); file_insert_char_at_cursor(file, '<'));
		KEY_PRESSED_OR_LONG_PRESSED(KEY_PERIOD, file_remove_selection(file); file_insert_char_at_cursor(file, '>'));
		KEY_PRESSED_OR_LONG_PRESSED(KEY_SLASH, file_remove_selection(file); file_insert_char_at_cursor(file, '?'));
		KEY_PRESSED_OR_LONG_PRESSED(KEY_GRAVE, file_remove_selection(file); file_insert_char_at_cursor(file, '~'));
	} else {
		KEY_PRESSED_OR_LONG_PRESSED(KEY_MINUS, file_remove_selection(file); file_insert_char_at_cursor(file, '-'));
		KEY_PRESSED_OR_LONG_PRESSED(KEY_EQUALS, file_remove_selection(file); file_insert_char_at_cursor(file, '='));
		KEY_PRESSED_OR_LONG_PRESSED(KEY_LEFTBRACKET, file_remove_selection(file); file_insert_char_at_cursor(file, '['));
		KEY_PRESSED_OR_LONG_PRESSED(KEY_RIGHTBRACKET, file_remove_selection(file); file_insert_char_at_cursor(file, ']'));
		KEY_PRESSED_OR_LONG_PRESSED(KEY_BACKSLASH, file_remove_selection(file); file_insert_char_at_cursor(file, '\\'));
		KEY_PRESSED_OR_LONG_PRESSED(KEY_SEMICOLON, file_remove_selection(file); file_insert_char_at_cursor(file, ';'));
		KEY_PRESSED_OR_LONG_PRESSED(KEY_APOSTROPHE, file_remove_selection(file); file_insert_char_at_cursor(file, '\''));
		KEY_PRESSED_OR_LONG_PRESSED(KEY_COMMA, file_remove_selection(file); file_insert_char_at_cursor(file, ','));
		KEY_PRESSED_OR_LONG_PRESSED(KEY_PERIOD, file_remove_selection(file); file_insert_char_at_cursor(file, '.'));
		KEY_PRESSED_OR_LONG_PRESSED(KEY_SLASH, file_remove_selection(file); file_insert_char_at_cursor(file, '/'));
		KEY_PRESSED_OR_LONG_PRESSED(KEY_GRAVE, file_remove_selection(file); file_insert_char_at_cursor(file, '`'));
	
		// Some numpad stuff
		KEY_PRESSED_OR_LONG_PRESSED(KEY_NUMDIVIDE, file_remove_selection(file); file_insert_char_at_cursor(file, '/'));
		KEY_PRESSED_OR_LONG_PRESSED(KEY_NUMMULTIPLY, file_remove_selection(file); file_insert_char_at_cursor(file, '*'));
		KEY_PRESSED_OR_LONG_PRESSED(KEY_NUMPLUS, file_remove_selection(file); file_insert_char_at_cursor(file, '+'));
		KEY_PRESSED_OR_LONG_PRESSED(KEY_NUMMINUS, file_remove_selection(file); file_insert_char_at_cursor(file, '-'));
		KEY_PRESSED_OR_LONG_PRESSED(KEY_NUMPERIOD, file_remove_selection(file); file_insert_char_at_cursor(file, '.'));

		KEY_PRESSED_OR_LONG_PRESSED(KEY_TAB, file_remove_selection(file); file_insert_char_at_cursor(file, '\t'));

		KEY_PRESSED_OR_LONG_PRESSED(KEY_SPACE, file_remove_selection(file); file_insert_char_at_cursor(file, ' '));
	}
}

void code_editor_update(computer_t *computer) {
	for (size_t i = 0; i < KEY_COUNT; i++) {
		if (_key_timers[i] > 0) {
			_key_timers[i]--;
		}
	}

	file_t *file = &computer->files[_current_file_index];

	// Cursor movement
	if (input_key_held(KEY_LCTRL) || input_key_held(KEY_RCTRL)) {
		KEY_PRESSED_OR_LONG_PRESSED(KEY_LEFT, {
			file_move_cursor_to_prev_token(file, true);
			_unblink_cursor();

			file->selection_start = file->cursor;
			file->selection_end = file->cursor;
		});

		KEY_PRESSED_OR_LONG_PRESSED(KEY_RIGHT, {
			file_move_cursor_to_next_token(file, true);
			_unblink_cursor();

			file->selection_start = file->cursor;
			file->selection_end = file->cursor;
		});
	} else {
		KEY_PRESSED_OR_LONG_PRESSED(KEY_LEFT, {
			if (file->selection_start.line == file->selection_end.line && file->selection_start.pos == file->selection_end.pos) {
				// There is no selection
				file_move_cursor_left(file);
			} else {
				// Move cursor to selection start
				file->cursor = file->selection_start;
			}

			_unblink_cursor();

			file->selection_start = file->cursor;
			file->selection_end = file->cursor;
		});

		KEY_PRESSED_OR_LONG_PRESSED(KEY_RIGHT, {
			if (file->selection_start.line == file->selection_end.line && file->selection_start.pos == file->selection_end.pos) {
				// There is no selection
				file_move_cursor_right(file);
			} else {
				// Move cursor to selection end
				file->cursor = file->selection_end;
			}

			_unblink_cursor();

			file->selection_start = file->cursor;
			file->selection_end = file->cursor;
		});
	}

	KEY_PRESSED_OR_LONG_PRESSED(KEY_UP, {
		file_move_cursor_up(file);
		_unblink_cursor();

		file->selection_start = file->cursor;
		file->selection_end = file->cursor;
	});

	KEY_PRESSED_OR_LONG_PRESSED(KEY_DOWN, {
		file_move_cursor_down(file);
		_unblink_cursor();

		file->selection_start = file->cursor;
		file->selection_end = file->cursor;
	});

	// TODO: page up, page down, home, end

	if (file->selection_start.line == file->selection_end.line && file->selection_start.pos == file->selection_end.pos) {
		KEY_PRESSED_OR_LONG_PRESSED(KEY_BACKSPACE, file_remove_char_at_cursor(file));
	} else {
		KEY_PRESSED_OR_LONG_PRESSED(KEY_BACKSPACE, file_remove_selection(file));
	}

	// Handle return
	KEY_PRESSED_OR_LONG_PRESSED(KEY_RETURN, {
		file_remove_selection(file);

		file_split_line_down(file, file->cursor.line, file->cursor.pos, 0);
		file->cursor.line++;
		file->cursor.pos = 0;
	});
	KEY_PRESSED_OR_LONG_PRESSED(KEY_NUMENTER, {
		file_remove_selection(file);
		
		file_split_line_down(file, file->cursor.line, file->cursor.pos, 0);
		file->cursor.line++;
		file->cursor.pos = 0;
	});

	_handle_char_input(computer, file);

	// Mouse
	if (input_mouse_button_pressed(MOUSE_BUTTON_LEFT)) {
		_move_cursor_to_mouse(computer->ram, file);
		_unblink_cursor();

		file->selection_start = file->cursor;
	}
	if (input_mouse_button_held(MOUSE_BUTTON_LEFT)) {
		_move_cursor_to_mouse(computer->ram, file);
		_unblink_cursor();

		file->selection_end = file->cursor;
	}
	
	if (input_mouse_button_released(MOUSE_BUTTON_LEFT)) {
		// Make start and end proper
		if (file->selection_end.line < file->selection_start.line || (file->selection_end.line == file->selection_start.line && file->selection_end.pos < file->selection_start.pos)) {
			file_pos_t temp = file->selection_start;
			file->selection_start = file->selection_end;
			file->selection_end = temp;
		}
	}

	// Copy
	if ((input_key_held(KEY_LCTRL) || input_key_held(KEY_RCTRL)) && input_key_pressed(KEY_C)) {
		if (!(file->selection_start.line == file->selection_end.line && file->selection_start.pos == file->selection_end.pos)) {
			string_t selection = file_get_selection_as_string(file, get_temp_allocator());
			set_clipboard_text(selection);
		}
	}

	// Paste
	if ((input_key_held(KEY_LCTRL) || input_key_held(KEY_RCTRL)) && input_key_pressed(KEY_V)) {
		file_insert_string_at_cursor(file, get_clipboard_text(get_temp_allocator()));
	}

	// Cut
	if ((input_key_held(KEY_LCTRL) || input_key_held(KEY_RCTRL)) && input_key_pressed(KEY_X)) {
		string_t selection = file_get_selection_as_string(file, get_temp_allocator());
		set_clipboard_text(selection);

		file_remove_selection(file);
	}

	// printf("selection: (%d %d), (%d %d)\n", file->selection_start.line, file->selection_start.pos, file->selection_end.line, file->selection_end.pos);

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

	// Draw buttons for files (this file is kinda dirty but it works)
	for (size_t i = 0; i < computer->active_files_amount; i++) {
		point_t pos = POINT(_layout.file_buttons_pos.x, _layout.file_buttons_pos.y + i * skin_layout.code_file_button.pressed_rect.h);
		
		if (gui_button(computer->ram, pos, skin_layout.code_file_button, _current_file_index == i)) {
			if (_current_file_index == i) {
				// TODO: see if I can't just replace this with a break statement
				goto ignore_current_file;
			}

			_current_file_index = i;
			
			// Remove empty tabs
			size_t last_index = computer->active_files_amount - 1;
			
			while (computer->active_files_amount > 1) {
				size_t last_index = computer->active_files_amount - 1;
				if (last_index <= _current_file_index) {
					break;
				}

				if (computer->files[last_index].line_amount == 0 || (computer->files[last_index].line_amount == 1 && computer->files[last_index].lines[0].string.len == 0)) {
					file_deinit(&computer->files[last_index]);
					computer->active_files_amount--;
					_current_file_index = MIN(_current_file_index, computer->active_files_amount - 1);
				} else {
					break;
				}
			}
		}

		ignore_current_file:

		gui_draw_string(computer->ram, CODE_EDITOR_FONT_INDEX, file_get_name(&computer->files[i]), POINT(pos.x + 3, pos.y + 3), COLOR_BLACK);
	}

	// TODO: don't also move cursor
	if (input_key_held(KEY_LALT) && input_key_pressed(KEY_UP)) {
		if (_current_file_index > 0) {
			_current_file_index--;
		}
	}
	if (input_key_held(KEY_LALT) && input_key_pressed(KEY_DOWN)) {
		if (_current_file_index < computer->active_files_amount - 1) {
			_current_file_index++;
		}
	}

	// Add file button
	if (computer->active_files_amount < FILES_AMOUNT) {
		if (gui_button(computer->ram, POINT(_layout.file_buttons_pos.x, _layout.file_buttons_pos.y + computer->active_files_amount * skin_layout.code_file_button.pressed_rect.h), skin_layout.add_file_button, false)) {
			file_append_line(&computer->files[computer->active_files_amount], STR(""));
			computer->active_files_amount++;
		}
	}

	// gui_inset_frame(computer->ram, _layout.code_rect);
	gfx_draw_filled_rect(fb_surf, _layout.code_rect, computer->ram->code_editor_config.background_color);

	// TODO: replace with temp alloc (maybe)
	char line_number_buffer[8];

	// Base x position of where every line is drawn on the screen
	const size_t line_x = _layout.code_rect.x + 2 + 5 * (font->widths[0] + font->horizontal_space);

	// Draw selection rect
	// This file looks like shit but it works for now
	file_t *file = &computer->files[_current_file_index];
	{
		file_pos_t start = file->selection_start;
		file_pos_t end = file->selection_end;

		// Make start and end proper
		if (end.line < start.line || (end.line == start.line && end.pos < start.pos)) {
			file_pos_t temp = start;
			start = end;
			end = temp;
		}

		if (start.line == end.line) {
			string_t string = computer->files[_current_file_index].lines[start.line].string;
	
			rect_t rect = {
				.x = line_x + gui_get_string_width(font, string_view(string, 0, start.pos), string.len),
				.y = _layout.code_rect.y + 2 + (start.line - _scroll_amount) * (font->height + font->vertical_space),
				.w = gui_get_string_width(font, string_view(string, start.pos, end.pos - start.pos), string.len),
				.h = font->height,
			};
	
			gfx_draw_filled_rect(fb_surf, rect, COLOR_CYAN);
	
		} else {
			for (size_t i = start.line; i <= end.line; i++) {
				string_t string = computer->files[_current_file_index].lines[i].string;
	
				if (i == start.line) {
					rect_t rect = {
						.x = line_x + gui_get_string_width(font, string_view(string, 0, start.pos), string.len),
						.y = _layout.code_rect.y + 2 + (i - _scroll_amount) * (font->height + font->vertical_space),
						.w = gui_get_string_width(font, string_view(string, start.pos, string.len - start.pos), string.len),
						.h = font->height,
					};
					gfx_draw_filled_rect(fb_surf, rect, COLOR_CYAN);
	
				} else if (i == end.line) {
					rect_t rect = {
						.x = line_x,
						.y = _layout.code_rect.y + 2 + (i - _scroll_amount) * (font->height + font->vertical_space),
						.w = gui_get_string_width(font, string_view(string, 0, end.pos), end.pos),
						.h = font->height,
					};
					gfx_draw_filled_rect(fb_surf, rect, COLOR_CYAN);
				} else {
					rect_t rect = {
						.x = line_x,
						.y = _layout.code_rect.y + 2 + (i - _scroll_amount) * (font->height + font->vertical_space),
						.w = gui_get_string_width(font, string, string.len),
						.h = font->height,
					};
					gfx_draw_filled_rect(fb_surf, rect, COLOR_CYAN);
				}
			}
		}
	}

	// TODO: fix font so I can refactor this hardcoded mess
	for (int i = 0; i < _lines_on_screen; i++) {
		if (i + _scroll_amount >= computer->files[_current_file_index].line_amount) {
			break;
		}

		// Commented out: version with leading zeroes
		// sprintf(line_number_buffer, "%04d", i + _scroll_amount + 1);
		sprintf(line_number_buffer, "% 4d", i + _scroll_amount + 1);

		// Line number
		// TODO: don't hardcode color
		gui_draw_text(computer->ram, CODE_EDITOR_FONT_INDEX, line_number_buffer, POINT(_layout.code_rect.x + 2, _layout.code_rect.y + 2 + (i * (font->height + font->vertical_space))), 8);
		
		// Line itself using tokens for syntax highlighting
		line_t *current_line = &computer->files[_current_file_index].lines[i + _scroll_amount];
		size_t current_x = line_x;

		for (size_t j = 0; j < current_line->tokens.len; j++) {
			color_t color = computer->ram->code_editor_config.token_colors[current_line->tokens.data[j].type];
			gui_draw_string(computer->ram, CODE_EDITOR_FONT_INDEX, current_line->tokens.data[j].string, POINT(current_x, _layout.code_rect.y + 2 + (i * (font->height + font->vertical_space))), color);
			current_x += gui_get_string_width(font, current_line->tokens.data[j].string, current_line->tokens.data[j].string.len);
		}
	}

	// Draw cursor
	if (_cursor_timer >= _cursor_blink_speed / 2) {
		int cursor_x = _layout.code_rect.x + 2 + 5 * (font->widths[0] + font->horizontal_space) + _get_real_cursor_pos(computer) - 1;
		int cursor_y = _layout.code_rect.y + 2 + computer->files[_current_file_index].cursor.line * (font->height + font->vertical_space) - _scroll_amount * (font->height + font->vertical_space);
		gfx_draw_line(fb_surf, POINT(cursor_x, cursor_y), POINT(cursor_x, cursor_y + font->height - 1), 3);
	}
	
	// Update cursor blink
	_cursor_timer--;
	if (_cursor_timer == 0) {
		_unblink_cursor();
	}
}
