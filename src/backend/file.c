#include "file.h"

#include <inttypes.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <assert.h>

#include "../common/math2d.h"

static void _compute_lines(file_t *file) {
	array_init(&file->edit_state.lines, get_heap_allocator());
	size_t last_start = 0;

	for (size_t i = 0; i < file->string.len; i++) {
		if (file->string.data[i] == '\n') {
			line_t line = {
				.start = last_start,
				.len = i - last_start,
			};
			array_push(&file->edit_state.lines, line);
			last_start = i + 1;
		}
	}

	// Last one
	{
		line_t line = {
			.start = last_start,
			.len = file->string.len - last_start,
		};
		array_push(&file->edit_state.lines, line);
	}
	
	// Print (for debugging)
	/*
	printf("lines amount: %d\n", file->edit_state.lines.len);
	for (size_t i = 0; i < file->edit_state.lines.len; i++) {
		printf("line start: %zu, len: %zu\n", file->edit_state.lines.data[i].start, file->edit_state.lines.data[i].len);
	}
	*/
}

static size_t _pos_to_line_index(file_t *file, size_t pos) {
	for (size_t i = 0; i < file->edit_state.lines.len; i++) {
		if (file->edit_state.lines.data[i].start > pos) {
			if (i > 0) {
				return i - 1;
			}
			return 0;
		}
	}

	return file->edit_state.lines.len - 1;
}

static void _trigger_on_edit(file_t *file) {
	_compute_lines(file);
	// TODO: compute tokens
}

// Resizs the string if necessary
static void _file_reserve(file_t *file, size_t needed_size) {
	if (file->capacity >= needed_size) {
		return;
	}

	size_t old_capacity = file->capacity;

	file->capacity = get_next_power_of_2(needed_size);

	void *new_data = alloc(get_heap_allocator(), file->capacity);
	memcpy(new_data, file->string.data, old_capacity);

	dealloc(get_heap_allocator(), file->string.data);

	file->string.data = new_data;
}

void file_insert_string_at(file_t *file, int pos, string_t text) {
	if (pos < 0) {
		return;
	}

	_file_reserve(file, file->string.len + text.len);
	memmove(file->string.data + pos + text.len, file->string.data + pos, file->string.len - pos);
	memcpy(file->string.data + pos, text.data, text.len);
	file->string.len += text.len;

	_trigger_on_edit(file);
}

void file_insert_char_at(file_t *file, int pos, char c) {
	if (pos < 0) {
		return;
	}

	_file_reserve(file, file->string.len + 1);
	memmove(file->string.data + pos + 1, file->string.data + pos, file->string.len - pos);
	file->string.data[pos] = c;
	file->string.len++;

	_trigger_on_edit(file);
}

void file_append_string(file_t *file, string_t string) {
	_file_reserve(file, file->string.len + string.len);
	memcpy(file->string.data + file->string.len, string.data, string.len);
	file->string.len += string.len;

	_trigger_on_edit(file);
}

void file_remove_section(file_t *file, int pos, size_t size) {
	if (pos < 0) {
		return;
	}
	
	// memmove(file->string.data - size, file->string.data, file->string.len - pos);
	memmove(file->string.data + pos, file->string.data + pos + size, file->string.len - pos);
	file->string.len -= size;
	_file_reserve(file, file->string.len - size);

	_trigger_on_edit(file);
}

void file_clear(file_t *file) {
	dealloc(get_heap_allocator(), file->string.data);
	file->string.data = NULL;
	file->string.len = 0;
	file->capacity = 0;
	file->edit_state.cursor_pos = 0;

	_trigger_on_edit(file);

	// array_deinit(&file->edit_state.lines);
}

size_t get_token_index_around_pos(file_t *file, int pos);

// TODO: doesnt seem to work if string below is empty
size_t file_move_pos_vertical(file_t *file, int pos, int64_t amount) {
	size_t line_index = _pos_to_line_index(file, pos);
	int64_t new_line_index =clamp_int(line_index + amount, 0, file->edit_state.lines.len - 1);
	
	line_t *line = &file->edit_state.lines.data[line_index];
	line_t *prev_line = &file->edit_state.lines.data[new_line_index];

	int64_t offset = pos - line->start;
	if (offset > prev_line->len) offset = prev_line->len;

	pos = prev_line->start + offset;

	return pos;
}

bool does_selection_exist(file_t *file) {
	return file->edit_state.selection_start != file->edit_state.selection_end;
}

string_t file_get_name(file_t *file) {
	string_t view = file->string;
	view.len = MIN(10, file->string.len);

	for (size_t i = 0; i < file->string.len; i++) {
		if (file->string.data[i] == '\n') {
			view.len = MIN(10, i);
			return view;
		}
	}

	return view;
}
