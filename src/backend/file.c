#include "file.h"

#include <inttypes.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <assert.h>

#include "../common/math2d.h"
#include "../api/api.h"

static void _compute_lines(file_t *file) {
	array_init(&file->edit_state.lines, get_heap_allocator());
	size_t last_start = 0;

	for (size_t i = 0; i < file->string.len; i++) {
		if (file->string.data[i] == '\n') {
			string_reference_t line = {
				.start = last_start,
				.len = i - last_start,
			};
			array_push(&file->edit_state.lines, line);
			last_start = i + 1;
		}
	}

	// Last one
	string_reference_t line = {
		.start = last_start,
		.len = file->string.len - last_start,
	};
	array_push(&file->edit_state.lines, line);
	
	// Print (for debugging)
	/*
	printf("lines amount: %d\n", file->edit_state.lines.len);
	for (size_t i = 0; i < file->edit_state.lines.len; i++) {
		printf("line start: %zu, len: %zu\n", file->edit_state.lines.data[i].start, file->edit_state.lines.data[i].len);
	}
	*/
}

// Doesn't contain "true", "false" and "nil" since those should be treated as literals by the lexer
static const char *lua_keywords[] = {
	"and", "break", "do", "else", "elseif", "end",
	"for", "function", "if", "in", "local",
	"not", "or", "repeat", "return", "then", "until", "while",
};

static bool _is_keyword(string_t word) {
	for (size_t i = 0; i < 18; i++) {
		if (string_eq(word, STR(lua_keywords[i]))) {
			return true;
		}
	}

	return false;
}

static bool _is_builtin_function(string_t word) {
	for (size_t i = 0; i < API_FUNC_COUNT; i++) {
		if (string_eq(word, STR(api_metas[i].name))) {
			return true;
		}
	}

	return false;
}

static bool _is_literal(string_t word) {
	return string_eq(word, STR("true")) || string_eq(word, STR("false")) || string_eq(word, STR("nil"));
}

static void _token_push(file_t *file, lua_token_type_t type, size_t start, size_t len) {
	lua_token_t token = {
		.type = type,
		.string_reference = (string_reference_t){
			.start = start,
			.len = len,
		},
	};

	array_push(&file->edit_state.tokens, token);
}

static void _print_token_list(file_t *file) {
	for (size_t i = 0; i < file->edit_state.tokens.len; i++) {
		switch (file->edit_state.tokens.data[i].type) {
			case LUA_TOKEN_KEYWORD: {
				printf("[KEYWORD] ");
				break;
			}
			case LUA_TOKEN_BUILTIN_FUNCTION: {
				printf("[BUILTIN_FUNCTION] ");
				break;
			}
			case LUA_TOKEN_IDENTIFIER: {
				printf("[IDENTIFIER] ");
				break;
			}
			case LUA_TOKEN_LITERAL: {
				printf("[LITERAL] ");
				break;
			}
			case LUA_TOKEN_STRING: {
				printf("[STRING] ");
				break;
			}
			case LUA_TOKEN_COMMENT: {
				printf("[COMMENT] ");
				break;
			}
			case LUA_TOKEN_OPERATOR: {
				printf("[OPERATOR] ");
				break;
			}
			case LUA_TOKEN_WHITESPACE: {
				printf("[WHITESPACE] ");
				break;
			}
		}

		printf("'");
		// TODO: fix
		// for (size_t j = 0; j < file->edit_state.tokens.data[i].string.len; j++) {
		// 	printf("%c", file->edit_state.tokens.data[i].string.data[j]);
		// }
		printf("'");

		printf("\n");
	}

	printf("\n");
}

// Update the token list on a given line
static void _compute_tokens(file_t *file) {
	size_t i = 0;

	// Reset token array
	array_init(&file->edit_state.tokens, get_heap_allocator());

	if (file->string.len == 0) {
		return;
	}

	// Loop line string
	while (i < file->string.len) {
		char c = file->string.data[i];

		// Keyword, builtin function, boolean literal, identifier
		if (is_alphabetic(c) || c == '_') {
			const size_t start = i;
			while (i < file->string.len && (is_alphanumeric(file->string.data[i]) || file->string.data[i] == '_')) {
				i++;
			}

			string_t word = string_view(file->string, start, i - start);

			if (_is_keyword(word)) {
				_token_push(file, LUA_TOKEN_KEYWORD, start, i - start);
			} else if (_is_builtin_function(word)) {
				_token_push(file, LUA_TOKEN_BUILTIN_FUNCTION, start, i - start);
			} else if (_is_literal(word)) {
				// If you think about it a bool is also a number :)
				_token_push(file, LUA_TOKEN_LITERAL, start, i - start);
			} else {
				_token_push(file, LUA_TOKEN_IDENTIFIER, start, i - start);
			}
		
		// Number
		} else if (is_digit(c)) {
			const size_t start = i;
			while (i < file->string.len && (is_digit(file->string.data[i]) || file->string.data[i] == '.')) {
				i++;
			}

			_token_push(file, LUA_TOKEN_LITERAL, start, i - start);
		
		// String
		} else if (file->string.data[i] == '"' || file->string.data[i] == '\'') {
			char quote_used = file->string.data[i];

			const size_t start = i;
			i++;

			while (i < file->string.len && (file->string.data[i] != quote_used)) {
				i++;
			}

			if (i < file->string.len) {
				i++;
			}

			_token_push(file, LUA_TOKEN_STRING, start, i - start);
		
		// Comment
		} else if (file->string.data[i] == '-' && (i + 1) < file->string.len && file->string.data[i + 1] == '-') {
			const size_t start = i;
			
			while (i < file->string.len && file->string.data[i] != '\n') {
				i++;
			}

			_token_push(file, LUA_TOKEN_COMMENT, start, i - start);

		// Whitespace
		} else if (is_whitespace(c)) {
			size_t start = i;
			while (i < file->string.len && (is_whitespace(file->string.data[i]))) {
				i++;
			}

			_token_push(file, LUA_TOKEN_WHITESPACE, start, i - start);
		
		// Operator, brackets
		} else {
			_token_push(file, LUA_TOKEN_OPERATOR, i, 1);

			i++;
		}
	}

	// For debugging
	// _print_token_list(file);
}

size_t file_get_line_index_from_pos(file_t *file, size_t pos) {
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
	_compute_tokens(file);
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
	
	memmove(file->string.data + pos, file->string.data + pos + size, file->string.len - pos);
	_file_reserve(file, file->string.len - size);
	file->string.len -= size;

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
	size_t line_index = file_get_line_index_from_pos(file, pos);
	int64_t new_line_index =clamp_int(line_index + amount, 0, file->edit_state.lines.len - 1);
	
	string_reference_t *line = &file->edit_state.lines.data[line_index];
	string_reference_t *prev_line = &file->edit_state.lines.data[new_line_index];

	int64_t offset = pos - line->start;
	if (offset > prev_line->len) offset = prev_line->len;

	pos = prev_line->start + offset;

	return pos;
}

bool file_does_selection_exist(file_t *file) {
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

// Swaps the selection_start and selection_end if necessary
void file_fix_selection(file_t *file) {
	if (file->edit_state.selection_start > file->edit_state.selection_end) {
		size_t temp = file->edit_state.selection_start;
		file->edit_state.selection_start = file->edit_state.selection_end;
		file->edit_state.selection_end = temp;
	}
}

void file_deselect(file_t *file) {
	file->edit_state.selection_start = file->edit_state.cursor_pos;
	file->edit_state.selection_end = file->edit_state.cursor_pos;
	// file->edit_state.selection_start = 0;
	// file->edit_state.selection_end = 0;
}
