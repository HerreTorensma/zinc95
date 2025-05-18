#include "text_file.h"

#include <inttypes.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>

const char *lua_keywords[] = {
	"and", "break", "do", "else", "elseif", "end",
	"false", "for", "function", "if", "in", "local", "nil",
	"not", "or", "repeat", "return", "then", "true", "until", "while",
};

typedef enum lua_token_type {
	LUA_TOKEN_KEYWORD,
	// Can be a number, boolean literal or nil
	LUA_TOKEN_LITERAL,
	LUA_TOKEN_OPERATOR,
	LUA_TOKEN_BUILTIN_FUNCTION,
	LUA_TOKEN_FUNCTION_NAME,
	LUA_TOKEN_FUNCTION_ARGUMENT,
} lua_token_type_t;

typedef struct lua_token {
	char *string;
	lua_token_type_t type;
} lua_token_t;

// Get the amount of lines in a string, used for loading
static size_t _string_get_lines_amount(const char *text) {
	size_t amount = 1;

	for (size_t i = 0; text[i] != '\0'; i++) {
		if (text[i] == '\n') {
			amount++;
		}
	}

	return amount;
}

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

// Add a new line to the data structure, used for loading a string before editing
// len is without null terminator (TODO: confirm this)
static void _file_add_line(file_t *file, const char *text, size_t len) {
	string_t *string = &file->lines[file->line_amount].string;

	string->data = malloc(len * sizeof(char));
	if (string->data == NULL) {
		printf("Couldn't allocate memory for new line\n");
		exit(EXIT_FAILURE);
	}

	memcpy(string->data, text, len);
	string->len = len;

	file->line_amount++;
}

// Load a string into the file_t datastructure
static void _string_to_file(file_t *file, const char *buffer) {
	size_t last_line_start = 0;
	size_t len = strlen(buffer);
	
	for (size_t i = 0; i < len; i++) {
		if (buffer[i] == '\n') {
			// - 1 so the \n is not included
			_file_add_line(file, buffer + last_line_start, i - last_line_start);
			last_line_start = i + 1;
		}
	}

	// Handle last line which might not have a newline char
	if (last_line_start < len) {
		_file_add_line(file, buffer + last_line_start, len - last_line_start);
	}
}

void file_load(file_t *file, const char *buffer) {
	// Apparantly calling free on a null pointer has no effect
	// so this is safe to do even if nothing is allocated yet
	file_free(file);

	// Allocate
	size_t lines_amount = _string_get_lines_amount(buffer);
	file->lines = malloc(lines_amount * sizeof(line_t));
	if (file->lines == NULL) {
		printf("Couldn't allocate memory for code\n");
		exit(EXIT_FAILURE);
	}

	_string_to_file(file, buffer);
}

// TODO: actually use this
size_t file_get_string_len(file_t *file) {
	size_t len = 0;

	for (size_t i = 0; i < file->line_amount; i++) {
		len += file->lines[i].string.len;
		
		// Add +1 for line break
		len++;
	}

	// Add +1 for null terminator
	len++;

	return len;
}

size_t file_to_string(file_t *file, char *buffer) {
	size_t offset = 0;

	for (size_t i = 0; i < file->line_amount; i++) {
		// size_t line_len = strlen(file->lines[i].text);
		string_t *string = &file->lines[i].string;
		
		// The +1 is for line break or null temrinator
		// memcpy(buffer + offset, file->lines[i].text, (line_len + 1) * sizeof(char));
		memcpy(buffer + offset, string->data, string->len * sizeof(char));
		
		if (i < file->line_amount - 1) {
			buffer[offset + string->len] = '\n';
		} else {
			buffer[offset + string->len] = '\0';
		}
		offset += string->len + 1;
	}

	return offset;
}

static size_t _len_at_pos(string_t *string, size_t pos) {
	return string->len - pos;
}

static void _move_lines_down(file_t *file, size_t line) {
	// Realloc lines
	line_t *temp = realloc(file->lines, (file->line_amount + 1ULL) * sizeof(line_t));
	if (temp == NULL) {
		printf("Couldn't realloc lines\n");
	}
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

static string_t _string_split(string_t *origin, size_t pos) {
	string_t second = {0};
	second.len = _len_at_pos(origin, pos);
	second.data = malloc(second.len * sizeof(char));
	memcpy(second.data, origin->data + pos, second.len * sizeof(char));

	origin->len = pos;
	// If origin->len is 0 then reallocing will free it
	// and it won't be malloced anywhere automatically
	// so we realloc to one byte to prevent crashes
	if (origin->len == 0) {
		// Give it one byte so we don't crash because of an unintended free
		origin->data = realloc(origin->data, 1);
	} else {
		origin->data = realloc(origin->data, origin->len * sizeof(char));
	}

	return second;
}

void file_split_line_down(file_t *file, size_t line, size_t pos, size_t indent_level) {
	// Make space for the new line
	_move_lines_down(file, line);
	
	file->lines[line + 1ULL].string = _string_split(&file->lines[line].string, pos);
	
	// TODO: Make this work (insert correct amount of tab characters)
	// if (strlen(file->lines[line + 1].text) == 0) {
	// 	for (int i = 0; i < indent_level; i++) {
	// 		file->lines[line + 1].text[i] = '\t';
	// 	}
	// }
}

// Assumes dest has enough memory for the concatenation
static void _string_concat(string_t *dest, string_t *src) {
	size_t old_len = dest->len;
	dest->len += src->len;
	dest->data = realloc(dest->data, dest->len * sizeof(char));

	memcpy(dest->data + old_len, src->data, src->len);
}

// Moves the lines below up by one, do the current line gets deleted
static void _move_lines_up(file_t *file, size_t line) {
	if (line < file->line_amount - 1) {
		// Move the lines up
		memmove(&file->lines[line], &file->lines[line + 1], (file->line_amount - line - 1ULL) * sizeof(line_t));
		// Realloc lines
		file->lines = realloc(file->lines, (file->line_amount - 1ULL) * sizeof(line_t));
	}

	// Decrement line amount
	file->line_amount--;
}

size_t file_merge_line_up(file_t *file, size_t line) {
	string_t *top_string = &file->lines[line - 1].string;
	string_t *bottom_string = &file->lines[line].string;
	size_t old_len = top_string->len;

	_string_concat(top_string, bottom_string);

	// Free the deleted line
	free(bottom_string->data);
	bottom_string->data = NULL;
	bottom_string->len = 0ULL;

	_move_lines_up(file, line);

	return old_len;
}

void file_insert_char_at(file_t *file, size_t line, size_t pos, char c) {
	// size_t len = strlen(file->lines[line].text);
	string_t *string = &file->lines[line].string;

	// Realloc and move line to make space for new character
	string->data = realloc(string->data, string->len + 1);
	memmove(string->data + pos + 1, string->data + pos, _len_at_pos(string, pos));
	
	// Increment length after moving so it doesnt do segfault
	string->len++;
	
	// Set character and update length
	file->lines[line].string.data[pos] = c;
}

void file_remove_char_at(file_t *file, size_t line, size_t pos) {
	if (pos == 0) {
		return;
	}

	string_t *string = &file->lines[line].string;

	memmove(string->data + pos - 1, string->data + pos, _len_at_pos(string, pos));
	string->len--;
	string->data = realloc(string->data, string->len);
}

void file_free(file_t *file) {
	for (size_t i = 0; i < file->line_amount; i++) {
		free(file->lines[i].string.data);
	}
	free(file->lines);
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