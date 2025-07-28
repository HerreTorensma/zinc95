#include "text_file.h"

#include <inttypes.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <assert.h>

#include "../api/api.h"

// TODO: investigate why stuff doesn't work when I have asserts after allocations
// because yes it works but it's also bad code and I should make it good
// they are commented out for now


// --- Tokenizer ---

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
	// return string_eq(word, (string_t)STR("true")) || string_eq(word, (string_t)STR("false"));
	return string_eq(word, STR("true")) || string_eq(word, STR("false")) || string_eq(word, STR("nil"));
}

static void _token_push(line_t *line, lua_token_type_t type, string_t string) {
	lua_token_t token = {
		.type = type,
		.string = string,
	};

	array_append(&line->tokens, token);
}

static void _print_token_list(line_t *line) {
	for (size_t i = 0; i < line->tokens.len; i++) {
		switch (line->tokens.data[i].type) {
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

		for (size_t j = 0; j < line->tokens.data[i].string.len; j++) {
			printf("%c", line->tokens.data[i].string.data[j]);
		}

		printf("\n");
	}

	printf("\n");
}

// Update the token list on a given line
static void _tokenize_line(file_t *file, size_t line_index) {
	line_t *line = &file->lines[line_index];
	string_t *string = &line->string;
	size_t i = 0;

	// Reset token array
	array_init(&line->tokens, get_heap_allocator());

	if (string->len == 0) {
		return;
	}

	// Loop line string
	while (i < string->len) {
		char c = string->data[i];

		// Keyword, builtin function, boolean literal, identifier
		if (is_alphabetic(c) || c == '_') {
			const size_t start = i;
			while (i < string->len && (is_alphanumeric(string->data[i]) || string->data[i] == '_')) {
				i++;
			}

			string_t word = string_view(*string, start, i - start);

			if (_is_keyword(word)) {
				_token_push(line, LUA_TOKEN_KEYWORD, word);
			} else if (_is_builtin_function(word)) {
				_token_push(line, LUA_TOKEN_BUILTIN_FUNCTION, word);
			} else if (_is_literal(word)) {
				// If you think about it a bool is also a number :)
				_token_push(line, LUA_TOKEN_LITERAL, word);
			} else {
				_token_push(line, LUA_TOKEN_IDENTIFIER, word);
			}
		
		// Number
		} else if (is_digit(c)) {
			const size_t start = i;
			while (i < string->len && (is_digit(string->data[i]) || string->data[i] == '.')) {
				i++;
			}

			string_t word = string_view(*string, start, i - start);


			_token_push(line, LUA_TOKEN_LITERAL, word);
		
		// String
		} else if (string->data[i] == '"' || string->data[i] == '\'') {
			char quote_used = string->data[i];

			size_t start = i;
			i++;

			while (i < string->len && (string->data[i] != quote_used)) {
				i++;
			}

			if (i < string->len) {
				i++;
			}

			string_t word = string_view(*string, start, i - start);

			_token_push(line, LUA_TOKEN_STRING, word);
		
		// Comment
		} else if (string->data[i] == '-' && (i + 1) < string->len && string->data[i + 1] == '-') {
			string_t word = string_view(*string, i, string->len - i);

			_token_push(line, LUA_TOKEN_COMMENT, word);

			break;
		
		// Whitespace
		} else if (is_whitespace(c)) {
			size_t start = i;
			while (i < string->len && (is_whitespace(string->data[i]))) {
				i++;
			}

			string_t word = string_view(*string, start, i - start);

			_token_push(line, LUA_TOKEN_WHITESPACE, word);
		
		// Operator, brackets
		} else {
			string_t word = string_view(*string, i, 1);

			_token_push(line, LUA_TOKEN_OPERATOR, word);

			i++;
		}
	}

	// For debugging
	// _print_token_list(line);
}



// --- Rest ---

// TODO: make this take a string and actually use it
int string_get_indent_level(const char text[]) {
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

void file_append_line(file_t *file, string_t line_view) {
	// Realloc
	file->lines = heap_realloc(file->lines, (file->line_amount + 1) * sizeof(line_t));

	// Zero-initialize
	memset(&file->lines[file->line_amount], 0, sizeof(line_t));

	file->lines[file->line_amount].string = string_copy(get_heap_allocator(), line_view);

	_tokenize_line(file, file->line_amount);

	file->line_amount++;
}

// Get the file as a string_t
string_t file_to_string(file_t *file, allocator_t allocator) {
	string_builder_t builder = {0};

	// 1 MB is probably enough (it will still resize if necessary)
	string_builder_init(&builder, allocator, MB(1));

	for (size_t i = 0; i < file->line_amount; i++) {
		string_builder_append(&builder, file->lines[i].string);
		string_builder_append(&builder, STR("\n"));
	}

	return builder.string;
}

static size_t _len_at_pos(string_t *string, size_t pos) {
	return string->len - pos;
}

static void _move_lines_down(file_t *file, size_t line) {
	// Realloc lines
	line_t *temp = heap_realloc(file->lines, (file->line_amount + 1ULL) * sizeof(line_t));
	// assert(temp != NULL && "Realloc failed");
	file->lines = temp;

	// Move the memory up
	size_t amount = file->line_amount - (size_t)line;
	memmove(&file->lines[line + 1ULL], &file->lines[line], amount * sizeof(line_t));
	// memcpy(&file->lines[line + 1ULL], &file->lines[line], amount * sizeof(line_t));

	// Zero-initialize the new line
	memset(&file->lines[line + 1ULL], 0, sizeof(line_t));
	
	// Increment line amount
	file->line_amount++;
}

// Edits the passed string and returns the new one
static string_t _string_split(string_t *origin, size_t pos) {
	string_t second = {0};
	second.len = _len_at_pos(origin, pos);
	second.data = heap_alloc(second.len * sizeof(char));
	// assert(second.data != NULL);

	memcpy(second.data, origin->data + pos, second.len * sizeof(char));

	origin->len = pos;
	// If origin->len is 0 then reallocing will free it
	// and it won't be malloced anywhere automatically
	// so we realloc to one byte to prevent crashes
	if (origin->len == 0) {
		// Give it one byte so we don't crash because of an unintended free
		origin->data = heap_realloc(origin->data, 1);
		// assert(origin->data != NULL && "Realloc failed");
	} else {
		origin->data = heap_realloc(origin->data, origin->len * sizeof(char));
		// assert(origin->data != NULL && "Realloc failed");
	}

	return second;
}

void file_split_line_down(file_t *file, size_t line, size_t pos, size_t indent_level) {
	// Make space for the new line
	_move_lines_down(file, line);
	
	// Zero initialize the new line
	memset(&file->lines[line + 1ULL], 0, sizeof(line_t));

	file->lines[line + 1ULL].string = _string_split(&file->lines[line].string, pos);

	// TODO: Make this work (insert correct amount of tab characters)
	// if (strlen(file->lines[line + 1].text) == 0) {
	// 	for (int i = 0; i < indent_level; i++) {
	// 		file->lines[line + 1].text[i] = '\t';
	// 	}
	// }

	_tokenize_line(file, line);
	_tokenize_line(file, line + 1ULL);
}

// Moves the lines below up by one, do the current line gets deleted
static void _move_lines_up(file_t *file, size_t line) {
	if (line < file->line_amount - 1) {
		// Move the lines up
		memmove(&file->lines[line], &file->lines[line + 1], (file->line_amount - line - 1ULL) * sizeof(line_t));
		// Realloc lines
		file->lines = heap_realloc(file->lines, (file->line_amount - 1ULL) * sizeof(line_t));
		// assert(file->lines != NULL && "Realloc failed");
	}

	// Decrement line amount
	file->line_amount--;
}

size_t file_merge_line_up(file_t *file, size_t line) {
	string_t *top_string = &file->lines[line - 1].string;
	string_t *bottom_string = &file->lines[line].string;
	size_t old_len = top_string->len;

	// _string_concat(top_string, bottom_string);

	string_t new_top_string = string_concat(get_heap_allocator(), *top_string, *bottom_string);
	heap_dealloc(top_string->data);
	heap_dealloc(bottom_string->data);
	*top_string = new_top_string;

	// Free the deleted line
	// free(bottom_string->data);
	// bottom_string->data = NULL;
	// bottom_string->len = 0ULL;

	// Free old tokens
	array_deinit(&file->lines[line].tokens);

	_move_lines_up(file, line);

	_tokenize_line(file, line - 1);

	return old_len;
}

void file_insert_char_at(file_t *file, size_t line, size_t pos, char c) {
	// size_t len = strlen(file->lines[line].text);
	string_t *string = &file->lines[line].string;

	// Realloc and move line to make space for new character
	string->data = heap_realloc(string->data, string->len + 1);
	// assert(string->data != NULL && "Realloc failed");

	memmove(string->data + pos + 1, string->data + pos, _len_at_pos(string, pos));
	
	// Increment length after moving so it doesnt do segfault
	string->len++;
	
	// Set character and update length
	file->lines[line].string.data[pos] = c;

	_tokenize_line(file, line);
}

void file_remove_char_at(file_t *file, size_t line, size_t pos) {
	if (pos == 0) {
		return;
	}

	string_t *string = &file->lines[line].string;

	memmove(string->data + pos - 1, string->data + pos, _len_at_pos(string, pos));
	string->len--;
	string->data = heap_realloc(string->data, string->len);
	// assert(string->data != NULL && "Realloc failed");

	_tokenize_line(file, line);
}

void file_deinit(file_t *file) {
	for (size_t i = 0; i < file->line_amount; i++) {
		if (file->lines[i].string.data != NULL) {
			free(file->lines[i].string.data);
			file->lines[i].string.data = NULL;
		}
	}
	
	if (file->lines != NULL) {
		free(file->lines);
		file->lines = NULL;
	}

	file->line_amount = 0;

	file->cursor_line = 0;
	file->cursor_pos = 0;
}

void file_insert_char_at_cursor(file_t *file, char c) {
	file_insert_char_at(file, file->cursor_line, file->cursor_pos, c);
	file->cursor_pos++;
}

void file_remove_char_at_cursor(file_t *file) {
	if (file->cursor_pos > 0) {
		file_remove_char_at(file, file->cursor_line, file->cursor_pos);
		file->cursor_pos--;
	} else {
		if (file->cursor_line > 0) {
			file->cursor_pos = file_merge_line_up(file, file->cursor_line);
			file->cursor_line--;
		}
	}
}

void file_move_cursor_up(file_t *file) {
	if (file->cursor_line > 0) {
		file->cursor_line--;

		size_t len = file->lines[file->cursor_line].string.len;
		if (file->cursor_pos > len) {
			file->cursor_pos = len;
		}
	}
}

void file_move_cursor_down(file_t *file) {
	if (file->cursor_line < file->line_amount - 1) {
		file->cursor_line++;
	}

	size_t len = file->lines[file->cursor_line].string.len;
	if (file->cursor_pos > len) {
		file->cursor_pos = len;
	}
}

void file_move_cursor_left(file_t *file) {
	if (file->cursor_pos > 0) {
		file->cursor_pos--;
	} else {
		if (file->cursor_line > 0) {
			file->cursor_line--;
			file->cursor_pos = file->lines[file->cursor_line].string.len;
		}
	}
}

void file_move_cursor_right(file_t *file) {
	size_t len = file->lines[file->cursor_line].string.len;
	if (file->cursor_pos < len) {
		file->cursor_pos++;
	} else {
		if (file->cursor_line < file->line_amount - 1) {
			file->cursor_line++;
			file->cursor_pos = 0;
		}
	}
}

static const char _divider_chars[] = {
	' ',
	'.',
	'(',
	')',
	';',
	',',
};

// TODO: use this
static bool _char_in_divider_chars(char c) {
	// No need to divide by the first element since we are working with chars which are 1 byte
	size_t len = sizeof(_divider_chars);

	for (size_t i = 0; i < len; i++) {
		if (c == _divider_chars[i]) {
			return true;
		}
	}

	return false;
}

void file_move_cursor_to_next_word(file_t *file) {
	int len = file->lines[file->cursor_line].string.len;
			
	for (size_t i = file->cursor_pos + 1; i < len + 1; i++) {
		if (file->lines[file->cursor_line].string.data[i] == ' ' || file->lines[file->cursor_line].string.data[i] == '.' || i == len) {
			file->cursor_pos = i;
			break;
		}
	}
}

void file_move_cursor_to_prev_word(file_t *file) {
	for (size_t i = file->cursor_pos - 1; i >= 0; i--) {
		if (file->lines[file->cursor_line].string.data[i] == ' ' || file->lines[file->cursor_line].string.data[i] == '.' || i == 0) {
			file->cursor_pos = i;
			break;
		}
	}
}

string_t file_get_name(file_t *file) {
	string_t string = file->lines[0].string;

	for (size_t i = 0; i < string.len; i++) {
		if (string.data[i] == '-' || string.data[i] == ' ' || string.data[i] == '\t') {
			continue;
		}

		return string_view(string, i, MIN(string.len - i, 10));
	}

	return string;
}
