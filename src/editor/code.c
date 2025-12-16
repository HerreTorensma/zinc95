#include "code.h"

#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#include "../backend/gfx.h"
#include "../backend/input.h"
#include "../backend/gui.h"
#include "../backend/file.h"
#include "../backend/window.h"
#include "../res.h"

#define LINE_NUMBER_DIGITS_AMOUNT 4

static struct {
	point_t file_buttons_pos;
	rect_t code_rect;
}
_layout = {
	.file_buttons_pos = {2, 22},
	.code_rect = {{68 + 2, 24 + 2, 568 - 4, 452 - 4}},
};

static size_t _current_file_index = 0;

static void _swap_if_a_greater_than_b(size_t *a, size_t *b) {
	if (*a > *b) {
		size_t temp = *a;
		*a = *b;
		*b = temp;
	}
}

void code_editor_init(computer_t *computer) {

}

static int _screen_pos_to_file_pos(file_t *file, font_t *font, rect_t rect, point_t pos, int tab_size, int line_number_digits_amount) {
	point_t adjusted_pos = {
		pos.x - rect.x + (font->widths[0] + font->horizontal_space) / 2,
		pos.y - rect.y + (file->edit_state.scroll_amount * (font->height + font->vertical_space)),
	};

	int line_index = adjusted_pos.y / (font->height + font->vertical_space);
	line_index = clamp_int(line_index, 0, file->edit_state.lines.len - 1);

	string_reference_t *line = &file->edit_state.lines.data[line_index];

	int visual_offset_from_line = (adjusted_pos.x / (font->widths[0] + font->horizontal_space)) - (line_number_digits_amount + 1);
	if (visual_offset_from_line < 0) {
		visual_offset_from_line = 0;
	}

	int offset_from_line = visual_string_pos_to_string_pos(string_view(file->string, line->start, line->len), visual_offset_from_line, tab_size);
	offset_from_line = clamp_int(offset_from_line, 0, line->len);

	int final_pos = clamp_int(line->start + offset_from_line, 0, file->string.len);

	return final_pos;
}

static void _commit_to_history(file_t *file, size_t start, size_t end, file_action_type_t type, bool was_selection) {
	_swap_if_a_greater_than_b(&start, &end);

	string_t view = string_view(file->string, start, end - start);
	
	string_t string = {0};

	if (start != end) {
		string = string_copy(get_heap_allocator(), view);
	}

	file_action_t action = {
		.pos = start,
		.string = string,
		.type = type,
		.was_selection = was_selection,
	};

	file->edit_state.history.allocator = get_heap_allocator();
	array_push(&file->edit_state.history, action);

	printf("pushed insert action\n");
}

static void _remove_selection_if_exists(file_t *file) {
	if (!file_does_selection_exist(file)) {
		return;
	}

	file_fix_selection(file);

	_commit_to_history(file, file->edit_state.selection_start, file->edit_state.selection_end, FILE_ACTION_REMOVE, true);

	file_remove_section(file, file->edit_state.selection_start, file->edit_state.selection_end - file->edit_state.selection_start);
	file->edit_state.cursor_pos = file->edit_state.selection_start;
	file_deselect(file);
	file->edit_state.supress_mouse_selection = true;
}

void _commit_pending_insert(file_t *file) {
	if (!file->edit_state.has_pending_insert)
		return;

	size_t start = file->edit_state.pending_insert_start;
	size_t end = file->edit_state.pending_insert_end;
	// _swap_if_a_greater_than_b(&start, &end); // Don't think this is necessary

	size_t len = end - start;

	if (len > 0) {
		file_action_t action = {
			.type = FILE_ACTION_INSERT,
			.pos = start,
			.string = string_copy(get_heap_allocator(), string_view(file->string, start, len)),
		};

		file->edit_state.history.allocator = get_heap_allocator();
		array_push(&file->edit_state.history, action);
	}

	file->edit_state.has_pending_insert = false;
}

// Returns true if a keybind was activated (so the char input can be skipped), false otherwise
static bool _handle_keybinds(computer_t *computer, file_t *file) {
	if (is_keybind_pressed(g_keybinds.global.undo)) {
		file_deselect(file);

		_commit_pending_insert(file);

		if (file->edit_state.history.len > 0) {
			file_action_t action = array_pop(&file->edit_state.history);
		
			if (action.type == FILE_ACTION_INSERT) {
				file_remove_section(file, action.pos, action.string.len);
				file->edit_state.cursor_pos = action.pos;
			} else if (action.type == FILE_ACTION_REMOVE) {
				file_insert_string_at(file, action.pos, action.string);
				file->edit_state.cursor_pos = action.pos + action.string.len;

				if (action.was_selection) {
					file->edit_state.selection_start = action.pos;
					file->edit_state.selection_end = action.pos + action.string.len;
				}
			}
		}

		return true;
	}

	if (is_keybind_pressed(g_keybinds.global.copy)) {
		file_fix_selection(file);
		
		if (file_does_selection_exist(file)) {
			string_t clipboard = string_view(file->string, file->edit_state.selection_start, file->edit_state.selection_end - file->edit_state.selection_start);
			set_clipboard_text(get_heap_allocator(), clipboard);
		} else {
			size_t line_index = file_get_line_index_from_pos(file, file->edit_state.cursor_pos);
			string_reference_t ref = file->edit_state.lines.data[line_index];
			string_t line = string_view(file->string, ref.start, ref.len);
			
			if (line.len == 0) {
				set_clipboard_text(get_temp_allocator(), STR("\n"));
				goto ah;
			}

			// TODO: find way to communicate this is a line and should be inserted below the current instead of in the middle (on paste)

			set_clipboard_text(get_heap_allocator(), line);
		}

		ah:

		return true;
	}

	if (is_keybind_pressed(g_keybinds.global.paste)) {
		_commit_pending_insert(file);

		_remove_selection_if_exists(file);

		string_t clipboard = get_clipboard_text(get_heap_allocator());
		file_insert_string_at(file, file->edit_state.cursor_pos, clipboard);
		
		_commit_to_history(file, file->edit_state.cursor_pos, file->edit_state.cursor_pos + clipboard.len, FILE_ACTION_INSERT, false);

		heap_dealloc(clipboard.data);
		file->edit_state.cursor_pos += clipboard.len;
		
		return true;
	}

	if (is_keybind_pressed(g_keybinds.global.cut)) {
		file_fix_selection(file);
		
		if (file_does_selection_exist(file)) {
			string_t clipboard = string_view(file->string, file->edit_state.selection_start, file->edit_state.selection_end - file->edit_state.selection_start);
			set_clipboard_text(get_heap_allocator(), clipboard);
			_remove_selection_if_exists(file);
		} else {
			size_t line_index = file_get_line_index_from_pos(file, file->edit_state.cursor_pos);
			string_reference_t ref = file->edit_state.lines.data[line_index];
			string_t line = string_view(file->string, ref.start, ref.len);

			if (line.len == 0) {
				set_clipboard_text(get_temp_allocator(), STR("\n"));
				goto eh;
			}

			// TODO: find way to communicate this is a line and should be inserted below the current instead of in the middle (on paste)

			set_clipboard_text(get_heap_allocator(), line);

			
			size_t start = clamp_int(ref.start - 1, 0, file->string.len);
			size_t len = clamp_int(ref.len + 1, 0, file->string.len);
			size_t end = clamp_int(ref.start + ref.len, 0, file->string.len);

			_commit_to_history(file, start, end, FILE_ACTION_REMOVE, false);
			file_remove_section(file, start, len);
			
			file->edit_state.cursor_pos = clamp_int(start + 1, 0, file->string.len);
		}

		eh:

		return true;
	}

	if (is_keybind_pressed(g_keybinds.global.select_all)) {
		file->edit_state.selection_start = 0;
		file->edit_state.selection_end = file->string.len - 1;
		file->edit_state.cursor_pos = file->string.len - 1;

		return true;
	}

	return false;
}

// TODO: split into multiple functions
// and make sure all the things don't intefere with each other
// So if one function returns some value that something happened the next one doesnt get executed
static void _file_update(computer_t *computer, file_t *file, rect_t rect, code_editor_config_t config) {
	bool any_keybind_executed = _handle_keybinds(computer, file);
	if (any_keybind_executed) {
		return;
	}

	// --- Keyboard ---

	// Cursor movement
	{
		bool shift_held = input_key_held(KEY_LSHIFT);
		bool cursor_moved = false;

		if (input_key_pressed_or_long_pressed(KEY_LEFT)) {
			if (file_does_selection_exist(file) && !shift_held) {
				file->edit_state.cursor_pos = file->edit_state.selection_start;
				file_deselect(file);
			} else {
				if (input_key_held(KEY_LCTRL)) {
					// Jump to previous word
					bool broke = false;

					for (int i = file->edit_state.cursor_pos - 2; i >= 0; i--) {
						if (file->string.data[i] == ' ' || file->string.data[i] == '\n' || file->string.data[i] == ',' || file->string.data[i] == '\t' || file->string.data[i] == '(') {
							file->edit_state.cursor_pos = i + 1;
							broke = true;
							break;
						}
					}

					if (!broke) {
						file->edit_state.cursor_pos = 0;
					}
				} else {
					file->edit_state.cursor_pos = clamp_int(file->edit_state.cursor_pos - 1, 0, file->string.len);
				}

				cursor_moved = true;
			}
		}
		if (input_key_pressed_or_long_pressed(KEY_RIGHT)) {
			if (file_does_selection_exist(file) && !shift_held) {
				file->edit_state.cursor_pos = file->edit_state.selection_end;
				file_deselect(file);
			} else {
				if (input_key_held(KEY_LCTRL)) {
					// Jump to next word
					bool broke = false;
					
					for (int i = file->edit_state.cursor_pos + 1; i < file->string.len; i++) {
						if (file->string.data[i] == ' ' || file->string.data[i] == '\n' || file->string.data[i] == ',' || file->string.data[i] == '\t' || file->string.data[i] == '(') {
							file->edit_state.cursor_pos = i;
							broke = true;
							break;
						}
					}

					if (!broke) {
						file->edit_state.cursor_pos = (file->string.len);
					}
				} else {
					file->edit_state.cursor_pos = clamp_int(file->edit_state.cursor_pos + 1, 0, file->string.len);
				}

				cursor_moved = true;
			}
		}

		// TODO: retain the original horizontal position of the cursor (decided only by left and right keys I think)
		// and move accordingly, rather than losing that information
		if (input_key_pressed_or_long_pressed(KEY_UP)) {
			if (file_does_selection_exist(file) && !shift_held) {
				file_deselect(file);
			}

			file->edit_state.cursor_pos = file_move_pos_vertical(file, file->edit_state.cursor_pos, -1);
			if (file_get_line_index_from_pos(file, file->edit_state.cursor_pos) < file->edit_state.scroll_amount) {
				file->edit_state.scroll_amount--;
			}

			cursor_moved = true;
		}
		if (input_key_pressed_or_long_pressed(KEY_DOWN)) {
			if (file_does_selection_exist(file) && !shift_held) {
				file_deselect(file);
			}

			file->edit_state.cursor_pos = file_move_pos_vertical(file, file->edit_state.cursor_pos, 1);
			font_t *font = &computer->ram->fonts[config.font_index];
			size_t lines_in_rect = rect.h / (font->height + font->vertical_space);
			if (file_get_line_index_from_pos(file, file->edit_state.cursor_pos) > (file->edit_state.scroll_amount + lines_in_rect)) {
				file->edit_state.scroll_amount++;
			}

			cursor_moved = true;
		}
	
		if (input_key_pressed_or_long_pressed(KEY_HOME)) {
			if (file_does_selection_exist(file) && !shift_held) {
				file_deselect(file);
			}

			size_t line_index = file_get_line_index_from_pos(file, file->edit_state.cursor_pos);
			file->edit_state.cursor_pos = file->edit_state.lines.data[line_index].start;

			cursor_moved = true;
		}
	
		if (input_key_pressed_or_long_pressed(KEY_END)) {
			if (file_does_selection_exist(file) && !shift_held) {
				file_deselect(file);
			}

			size_t line_index = file_get_line_index_from_pos(file, file->edit_state.cursor_pos);
			file->edit_state.cursor_pos = file->edit_state.lines.data[line_index].start + file->edit_state.lines.data[line_index].len;

			cursor_moved = true;
		}
	
		if (shift_held && cursor_moved) {
			file->edit_state.selection_end = file->edit_state.cursor_pos;
		}
	
		if (input_key_pressed(KEY_LSHIFT)) {
			if (file->edit_state.selection_start != file->edit_state.cursor_pos && file->edit_state.selection_end != file->edit_state.cursor_pos) {
				file_deselect(file);
			}
		}
		
		if (input_key_released(KEY_LSHIFT)) {
			file_fix_selection(file);
		}
	}

	if (input_key_pressed_or_long_pressed(KEY_BACKSPACE)) {
		printf("selection start: %d, selection end: %d\n", file->edit_state.selection_start, file->edit_state.selection_end);

		_commit_pending_insert(file);

		if (file_does_selection_exist(file)) {
			_remove_selection_if_exists(file);
		} else {
			_commit_to_history(file, file->edit_state.cursor_pos - 1, file->edit_state.cursor_pos, FILE_ACTION_REMOVE, false);

			file_remove_section(file, file->edit_state.cursor_pos - 1, 1);
			file->edit_state.cursor_pos = clamp_int(file->edit_state.cursor_pos - 1, 0, file->string.len);
		}
	}

	// --- Letters, digits, characters etc.
	{
		// TODO: caps lock
		char c = input_get_as_char();
		if (!(c == '\0' || c == '\b')) {
			if (!file->edit_state.has_pending_insert) {
				file->edit_state.pending_insert_start = file->edit_state.cursor_pos;
				file->edit_state.has_pending_insert = true;
			}

			// TODO: put removing selection and inserting character in one commit?
			// That's hard to do with the current system though
			// But for the moving of a selection or line I'll also need it
			// So one action can do both
			_remove_selection_if_exists(file);

			size_t line_index = file_get_line_index_from_pos(file, file->edit_state.cursor_pos);
			string_reference_t ref = file->edit_state.lines.data[line_index];
			string_t line = string_view(file->string, ref.start, ref.len);
			
			file_insert_char_at(file, file->edit_state.cursor_pos, c);
			file->edit_state.cursor_pos++;

			// Match indentation of current line
			if (c == '\n') {
				size_t indentation_level = 0;

				for (size_t i = 0; i < line.len; i++) {
					if (line.data[i] == '\t') {
						indentation_level++;
					} else {
						break;
					}
				}

				for (size_t i = 0; i < indentation_level; i++) {
					file_insert_char_at(file, file->edit_state.cursor_pos, '\t');
					file->edit_state.cursor_pos++;
				}
			}

			file->edit_state.pending_insert_end = file->edit_state.cursor_pos;

			if (is_whitespace(c)) {
				_commit_pending_insert(file);
			}
		}
	}

	// --- Mouse ---
	{
		point_t mouse_pos = input_get_mouse_pos();
	
		if (point_in_rect(mouse_pos, rect)) {
			input_set_cursor_style(CURSOR_STYLE_TEXT);
		}
	
		if (input_mouse_scrolled(SCROLL_DIR_DOWN)) {
			file->edit_state.scroll_amount += config.scroll_speed;
		}
		if (input_mouse_scrolled(SCROLL_DIR_UP)) {
			file->edit_state.scroll_amount -= config.scroll_speed;
		}
		file->edit_state.scroll_amount = clamp_int(file->edit_state.scroll_amount, 0, file->edit_state.lines.len - 1);
	
		if (point_in_rect(input_get_mouse_pos(), rect)) {
			if (input_mouse_button_pressed(MOUSE_BUTTON_LEFT)) {
				font_t *font = &computer->ram->fonts[config.font_index];
				file->edit_state.cursor_pos = _screen_pos_to_file_pos(file, font, rect, input_get_mouse_pos(), config.tab_size, LINE_NUMBER_DIGITS_AMOUNT);
				file->edit_state.selection_start = file->edit_state.cursor_pos;
			}
		
			if (!file->edit_state.supress_mouse_selection && input_mouse_button_held(MOUSE_BUTTON_LEFT)) {
				font_t *font = &computer->ram->fonts[config.font_index];
				file->edit_state.cursor_pos = _screen_pos_to_file_pos(file, font, rect, input_get_mouse_pos(), config.tab_size, LINE_NUMBER_DIGITS_AMOUNT);
				file->edit_state.selection_end = file->edit_state.cursor_pos;
			}
		
			if (input_mouse_button_released(MOUSE_BUTTON_LEFT)) {
				file_fix_selection(file);
				file->edit_state.supress_mouse_selection = false;
			}
		}
	}
}

void code_editor_update(computer_t *computer) {
	_file_update(computer, &computer->files[_current_file_index], _layout.code_rect, computer->ram->code_editor_config);
}

static point_t _file_cursor_pos_to_screen_pos(computer_t *computer, file_t *file, font_t *font, code_editor_config_t config) {
	// point_t pos = POINT(rect.x + file->edit_state.cursor_pos * (font->widths[0] + font->horizontal_space), rect.y);
	// point_t pos = POINT(rect.x + file->edit_state.cursor_pos * (font->widths[0] + font->horizontal_space), rect.y);
	point_t pos = {0};

	if (file->string.len == 0) {
		return pos;
	}
	
	for (size_t i = 0; i < file->edit_state.cursor_pos; i++) {
		if (file->string.data[i] == '\n') {
			pos.y += font->height + font->vertical_space;
			pos.x = 0;
			continue;
		}

		if (file->string.data[i] == '\t') {
			pos.x += (font->widths[0] + font->horizontal_space) * config.tab_size;
			continue;
		}

		pos.x += font->widths[0] + font->horizontal_space;
	}

	return pos;
}

static void _draw_selection_rect_for_char(computer_t *computer, file_t *file, font_t *font, int x, int y, int width_in_chars, color_t color, int index, rect_t rect) {
	// Don't draw if offscreen
	if (y < rect.y || y >= rect.y + rect.h) {
		return;
	}
	
	size_t temp_selection_start = file->edit_state.selection_start;
	size_t temp_selection_end = file->edit_state.selection_end;
	_swap_if_a_greater_than_b(&temp_selection_start, &temp_selection_end);
	if (file_does_selection_exist(file) && index >= temp_selection_start && index < temp_selection_end) {
		gfx_draw_filled_rect(FB_SURF(computer->ram->framebuffer.data), RECT(x, y, (font->widths[0] + font->horizontal_space) * width_in_chars, font->height + font->vertical_space), color);
	}
}

static void _file_draw(computer_t *computer, file_t *file, rect_t rect, code_editor_config_t config) {
	font_t *font = &computer->ram->fonts[config.font_index];

	size_t lines_in_rect = rect.h / (font->height + font->vertical_space);

	int text_start_x = rect.x + ((LINE_NUMBER_DIGITS_AMOUNT + 1) * (font->widths[0] + font->horizontal_space));

	// Line numbers
	{
		for (size_t i = 0; i <= lines_in_rect; i++) {
			size_t line_index = file->edit_state.scroll_amount + i;
			if (line_index >= file->edit_state.lines.len) {
				break;
			}

			string_t line_number_string = temp_alloc_string(8);
			// TODO: do another sprintf to make the format string? so the amount of digits is configurable
			sprintf(line_number_string.data, "% 4d", (int)(line_index + 1));
			line_number_string.len = strlen(line_number_string.data);

			gui_draw_string(computer->ram, config.font_index, line_number_string, POINT(rect.x, rect.y + i * (font->height + font->vertical_space)), config.line_number_color);
		}
	}

	// Text
	{
		int new_x = text_start_x;
		int new_y = rect.y - (file->edit_state.scroll_amount * (font->height + font->horizontal_space));
	
		size_t line_index = 0;
	
		for (size_t i = 0; i < file->edit_state.tokens.len; i++) {
			string_reference_t string_reference = file->edit_state.tokens.data[i].string_reference;
			string_t string = string_view(file->string, string_reference.start, string_reference.len);
	
			for (size_t j = 0; j < string.len; j++) {
				// Commented this out for now, might add it back later not sure yet
				if (string.data[j] == '\n') {
					// Selection
					_draw_selection_rect_for_char(computer, file, font, new_x, new_y, 1, config.selection_color, string_reference.start + j, rect);
	
					new_x = text_start_x;
					new_y += font->height + font->vertical_space;
					line_index++;
	
					continue;
				}
	
				// Clipping
				if (line_index < file->edit_state.scroll_amount || line_index > file->edit_state.scroll_amount + lines_in_rect) {
					continue;
				}
	
				_draw_selection_rect_for_char(computer, file, font, new_x, new_y, string.data[j] == '\t' ? config.tab_size : 1, config.selection_color, string_reference.start + j, rect);
	
				// Get the correct sprite index keeping in mind some fonts could have multiple sprites per character (not tested for more than 1 horizontal sprite)
				// int char_index = string.data[j] == '\t' ? 127 : string.data[j] - VISIBLE_CHARACTERS_START;
				int char_index = string.data[j] == '\t' ? 127 - VISIBLE_CHARACTERS_START : string.data[j] - VISIBLE_CHARACTERS_START;
				int x_offset = (char_index % (SPRITES_PER_ROW / font->sprite_width)) * font->sprite_width;
				int y_offset = (char_index / (SPRITES_PER_ROW / font->sprite_width)) * font->sprite_height;
				int sprite_index = font->sprite_index + x_offset + (y_offset * SPRITES_PER_ROW);
	
				rect_t rect = sprite_index_to_spritesheet_rect(sprite_index, font->sprite_width, font->sprite_height);
	
				for (int k = 0; k < rect.h; k++) {
					for (int l = 0; l < rect.w; l++) {
						color_t pixel_color = gfx_spritesheet_get_pixel(&computer->ram->spritesheet, (point_t){rect.x + l, rect.y + k});
	
						if (pixel_color != font->color_key && pixel_color != font->seperator_color) {
							gfx_set_pixel(&computer->ram->framebuffer, new_x + l, new_y + k, config.token_colors[file->edit_state.tokens.data[i].type]);
						}
					}
				}
	
				if (string.data[j] == '\t') {
					new_x += (font->widths[0] + font->horizontal_space) * config.tab_size;
 				} else {
					new_x += font->widths[0] + font->horizontal_space;
				}
			}
		}
	}

	// Cursor
	{
		point_t start_pos = _file_cursor_pos_to_screen_pos(computer, file, font, config);
		start_pos.x += text_start_x;
		start_pos.y += rect.y;
		start_pos.x -= 1; // So the cursor doesnt overwrite the leftmost pixels of the character right of it
		start_pos.y -= file->edit_state.scroll_amount * (font->height + font->vertical_space);
		
		if (start_pos.y >= rect.y && start_pos.y < rect.y + rect.h) { // Clipping
			gfx_draw_line(FB_SURF(computer->ram->framebuffer.data), start_pos, POINT(start_pos.x, start_pos.y + font->height), config.cursor_color);
		}
	}
}

static void _draw_file_buttons(computer_t *computer) {
	// Draw buttons for files (this file is kinda dirty but it works)
	for (size_t i = 0; i < computer->active_files_amount; i++) {
		point_t pos = POINT(_layout.file_buttons_pos.x, _layout.file_buttons_pos.y + i * g_skin_layout.code_file_button.pressed_rect.h);
		
		if (gui_button(computer->ram, pos, g_skin_layout.code_file_button, _current_file_index == i)) {
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

				if (computer->files[last_index].string.len == 0) {
					file_clear(&computer->files[last_index]);
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
	// if (input_key_held(KEY_LALT) && input_key_pressed(KEY_UP)) {
	// 	if (_current_file_index > 0) {
	// 		_current_file_index--;
	// 	}
	// }
	// if (input_key_held(KEY_LALT) && input_key_pressed(KEY_DOWN)) {
	// 	if (_current_file_index < computer->active_files_amount - 1) {
	// 		_current_file_index++;
	// 	}
	// }

	// + button
	if (computer->active_files_amount < FILES_AMOUNT) {
		if (gui_button(computer->ram, POINT(_layout.file_buttons_pos.x, _layout.file_buttons_pos.y + computer->active_files_amount * g_skin_layout.code_file_button.pressed_rect.h), g_skin_layout.add_file_button, false)) {
			file_append_string(&computer->files[computer->active_files_amount], STR(""));
			computer->active_files_amount++;
		}
	}
}

void code_editor_draw(computer_t *computer) {
	_file_draw(computer, &computer->files[_current_file_index], _layout.code_rect, computer->ram->code_editor_config);
	_draw_file_buttons(computer);
}
