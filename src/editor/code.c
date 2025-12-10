#include "code.h"

#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#include "../backend/gfx.h"
#include "../backend/input.h"
#include "../backend/gui.h"
#include "../backend/file.h"
#include "../backend/window.h"

typedef struct layout {
	point_t file_buttons_pos;
	rect_t code_rect;
} layout_t;

static const layout_t _layout = {
	.file_buttons_pos = {2, 22},
	.code_rect = {{68 + 2, 24 + 2, 568 - 4, 452 - 4}},
	// .code_rect = {{68 + 2 + 50, 24 + 2 + 50, 568 - 4 - 50, 452 - 4 - 50}},
};

static size_t _current_file_index = 0; 

void code_editor_init(computer_t *computer) {

}

static int _screen_pos_to_file_pos(file_t *file, font_t *font, rect_t rect, point_t pos, int tab_size) {
	point_t adjusted_pos = {
		pos.x - rect.x + (font->widths[0] + font->horizontal_space) / 2,
		pos.y - rect.y + (file->edit_state.scroll_amount * (font->height + font->vertical_space)),
	};

	int line_index = adjusted_pos.y / (font->height + font->vertical_space);
	line_index = clamp_int(line_index, 0, file->edit_state.lines.len - 1);

	string_reference_t *line = &file->edit_state.lines.data[line_index];

	int offset_from_line_with_tabs_expanded = adjusted_pos.x / (font->widths[0] + font->horizontal_space);

	int offset_from_line = offset_from_line_with_tabs_expanded;
	{
		string_t thing = string_view(file->string, line->start, offset_from_line);
		
		for (size_t i = 0; i < thing.len; i++) {
			if (thing.data[i] == '\t') {
				offset_from_line -= (tab_size - 1);
			}
		}
		
		offset_from_line = clamp_int(offset_from_line, 0, line->len);
	}

	int final_pos = clamp_int(line->start + offset_from_line, 0, file->string.len);

	return final_pos;
}

static void _remove_selection_if_exists(file_t *file) {
	if (file_does_selection_exist(file)) {
		file_fix_selection(file);

		file_remove_section(file, file->edit_state.selection_start, file->edit_state.selection_end - file->edit_state.selection_start);
		file->edit_state.cursor_pos = file->edit_state.selection_start;
		file_deselect(file);
		file->edit_state.supress_mouse_selection = true;
	}
}

// TODO: split into multiple functions
// and make sure all the things don't intefere with each other
// So if one function returns some value that something happened the next one doesnt get executed
static void _file_update(computer_t *computer, file_t *file, rect_t rect, code_editor_config_t config) {
	// --- Keyboard ---
	char c = input_get_as_char();
	if (!(c == '\0' || c == '\b')) {
		_remove_selection_if_exists(file);
		
		file_insert_char_at(file, file->edit_state.cursor_pos, c);
		file->edit_state.cursor_pos++;
	}

	if (input_key_pressed_or_long_pressed(KEY_LEFT)) {
		if (input_key_held(KEY_LSHIFT)) {
			file->edit_state.cursor_pos = clamp_int(file->edit_state.cursor_pos - 1, 0, file->string.len);
			file->edit_state.selection_end = file->edit_state.cursor_pos;
		} else {
			if (!file_does_selection_exist(file)) {
				file->edit_state.cursor_pos = clamp_int(file->edit_state.cursor_pos - 1, 0, file->string.len);
			}
			file_deselect(file);
		}
		printf("cursor pos: %d\n", file->edit_state.cursor_pos);
	}
	if (input_key_pressed_or_long_pressed(KEY_RIGHT)) {
		if (input_key_held(KEY_LSHIFT)) {
			file->edit_state.cursor_pos = clamp_int(file->edit_state.cursor_pos + 1, 0, file->string.len);
			file->edit_state.selection_end = file->edit_state.cursor_pos;
		} else {
			if (!file_does_selection_exist(file)) {
				file->edit_state.cursor_pos = clamp_int(file->edit_state.cursor_pos + 1, 0, file->string.len);
			}
			file_deselect(file);
		}
		printf("cursor pos: %d\n", file->edit_state.cursor_pos);
	}

	if (input_key_pressed(KEY_LSHIFT)) {
		file->edit_state.selection_start = file->edit_state.cursor_pos;
		file->edit_state.selection_end = file->edit_state.cursor_pos;
	}
	if (input_key_released(KEY_LSHIFT)) {
		file_fix_selection(file);
	}

	if (input_key_pressed_or_long_pressed(KEY_UP)) {
		file->edit_state.cursor_pos = file_move_pos_vertical(file, file->edit_state.cursor_pos, -1);
		printf("cursor pos: %d\n", file->edit_state.cursor_pos);

		if (input_key_held(KEY_LSHIFT)) {
			file->edit_state.selection_end = file->edit_state.cursor_pos;
		} else {
			file_deselect(file);
		}
	}
	if (input_key_pressed_or_long_pressed(KEY_DOWN)) {
		file->edit_state.cursor_pos = file_move_pos_vertical(file, file->edit_state.cursor_pos, 1);
		printf("cursor pos: %d\n", file->edit_state.cursor_pos);

		if (input_key_held(KEY_LSHIFT)) {
			file->edit_state.selection_end = file->edit_state.cursor_pos;
		} else {
			file_deselect(file);
		}
	}

	if (input_key_held(KEY_LCTRL) || input_key_held(KEY_RCTRL)) {
		// Copy
		if (input_key_pressed(KEY_C)) {
			file_fix_selection(file);
			string_t clipboard = string_view(file->string, file->edit_state.selection_start, file->edit_state.selection_end - file->edit_state.selection_start);
			set_clipboard_text(get_heap_allocator(), clipboard);
		}
		
		// Paste
		if (input_key_pressed(KEY_V)) {
			string_t clipboard = get_clipboard_text(get_heap_allocator());
			printf("clipboard len: %zu\n", clipboard.len);
			file_insert_string_at(file, file->edit_state.cursor_pos, clipboard);
			heap_dealloc(clipboard.data);
			file->edit_state.cursor_pos += clipboard.len;
		}

		// Select all
		if (input_key_pressed(KEY_A)) {
			file->edit_state.selection_start = 0;
			file->edit_state.selection_end = file->string.len - 1;
			file->edit_state.cursor_pos = file->string.len - 1;
		}
	}

	if (input_key_pressed_or_long_pressed(KEY_BACKSPACE)) {
		printf("selection start: %d, selection end: %d\n", file->edit_state.selection_start, file->edit_state.selection_end);
		
		if (file_does_selection_exist(file)) {
			_remove_selection_if_exists(file);
		} else {
			file_remove_section(file, file->edit_state.cursor_pos - 1, 1);
			file->edit_state.cursor_pos = clamp_int(file->edit_state.cursor_pos - 1, 0, file->string.len);
		}
	}

	// --- Mouse ---
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
			file->edit_state.cursor_pos = _screen_pos_to_file_pos(file, font, rect, input_get_mouse_pos(), config.tab_size);
			file->edit_state.selection_start = file->edit_state.cursor_pos;
		}
	
		if (!file->edit_state.supress_mouse_selection && input_mouse_button_held(MOUSE_BUTTON_LEFT)) {
			font_t *font = &computer->ram->fonts[config.font_index];
			file->edit_state.cursor_pos = _screen_pos_to_file_pos(file, font, rect, input_get_mouse_pos(), config.tab_size);
			file->edit_state.selection_end = file->edit_state.cursor_pos;
		}
	
		if (input_mouse_button_released(MOUSE_BUTTON_LEFT)) {
			file_fix_selection(file);
			file->edit_state.supress_mouse_selection = false;
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

static void _if_a_bigger_swap(size_t *a, size_t *b) {
	if (*a > *b) {
		size_t temp = *a;
		*a = *b;
		*b = temp;
	}
}

static void _draw_selection_rect_for_char(computer_t *computer, file_t *file, font_t *font, int x, int y, int width_in_chars, color_t color, int index) {
	size_t temp_selection_start = file->edit_state.selection_start;
	size_t temp_selection_end = file->edit_state.selection_end;
	_if_a_bigger_swap(&temp_selection_start, &temp_selection_end);
	if (file_does_selection_exist(file) && index >= temp_selection_start && index < temp_selection_end) {
		gfx_draw_filled_rect(FB_SURF(computer->ram->framebuffer.data), RECT(x, y, (font->widths[0] + font->horizontal_space) * width_in_chars, font->height + font->vertical_space), color);
	}
}

static void _file_draw(computer_t *computer, file_t *file, rect_t rect, code_editor_config_t config) {
	font_t *font = &computer->ram->fonts[config.font_index];

	size_t lines_in_rect = rect.h / (font->height + font->vertical_space);

	// Text
	{
		int new_x = rect.x;
		int new_y = rect.y - (file->edit_state.scroll_amount * (font->height + font->horizontal_space));
	
		size_t line_index = 0;
	
		size_t global_index = 0;
	
		for (size_t i = 0; i < file->edit_state.tokens.len; i++) {
			string_reference_t string_reference = file->edit_state.tokens.data[i].string_reference;
			string_t string = string_view(file->string, string_reference.start, string_reference.len);
	
			for (size_t j = 0; j < string.len; j++) {
				// Commented this out for now, might add it back later not sure yet
				if (string.data[j] == '\n') {
					// Selection
					_draw_selection_rect_for_char(computer, file, font, new_x, new_y, 1, config.selection_color, string_reference.start + j);
	
					new_x = rect.x;
					new_y += font->height + font->vertical_space;
					line_index++;
	
					if (j != file->edit_state.cursor_pos - 1) {
						_draw_selection_rect_for_char(computer, file, font, new_x, new_y, 1, config.selection_color, string_reference.start + j);
	
					}
					continue;
				}
	
				// Clipping
				if (line_index < file->edit_state.scroll_amount || line_index > file->edit_state.scroll_amount + lines_in_rect) {
					continue;
				}
	
				if (string.data[j] == '\t') {
					_draw_selection_rect_for_char(computer, file, font, new_x, new_y, config.tab_size, config.selection_color, string_reference.start + j);
	
					new_x += (font->widths[' ' - VISIBLE_CHARACTERS_START] + font->horizontal_space) * config.tab_size;
	
					continue;
				}
	
				_draw_selection_rect_for_char(computer, file, font, new_x, new_y, 1, config.selection_color, string_reference.start + j);
	
				// Get the correct sprite index keeping in mind some fonts could have multiple sprites per character (not tested for more than 1 horizontal sprite)
				int char_index = string.data[j] - VISIBLE_CHARACTERS_START;
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
	
				new_x += font->widths[string.data[j] - VISIBLE_CHARACTERS_START] + font->horizontal_space;
			}
		}
	}

	// Cursor
	{
		point_t start_pos = _file_cursor_pos_to_screen_pos(computer, file, font, config);
		start_pos.x += rect.x;
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

	// + button
	if (computer->active_files_amount < FILES_AMOUNT) {
		if (gui_button(computer->ram, POINT(_layout.file_buttons_pos.x, _layout.file_buttons_pos.y + computer->active_files_amount * skin_layout.code_file_button.pressed_rect.h), skin_layout.add_file_button, false)) {
			file_append_string(&computer->files[computer->active_files_amount], STR(""));
			computer->active_files_amount++;
		}
	}
}

void code_editor_draw(computer_t *computer) {
	_file_draw(computer, &computer->files[_current_file_index], _layout.code_rect, computer->ram->code_editor_config);
	_draw_file_buttons(computer);
}
