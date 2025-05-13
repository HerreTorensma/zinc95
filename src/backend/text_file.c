#include "text_file.h"

#include <inttypes.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

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
size_t string_get_lines_amount(const char *text) {
	size_t amount = 0;

	for (size_t i = 0; i < strlen(text); i++) {
		if (text[i] == '\n' || text[i] == '\0') {
			amount++;
		}
	}

	amount++;

	return amount;
}

// Append a new line, used for loading a string before editing
void line_append(file_t *code, const char *text, size_t len) {
	code->lines[code->line_amount].text = malloc((len + 1) * sizeof(char));
	if (code->lines[code->line_amount].text == NULL) {
		printf("Couldn't allocate memory for new line\n");
		exit(1);
	}

	memcpy(code->lines[code->line_amount].text, text, len);

	code->lines[code->line_amount].text[len] = '\0';
	code->line_amount++;
}

// Load a string into the file_t datastructure
void string_to_code(file_t *code, char *text) {
	char *last_line_start = text;
	size_t pos_since_last_line_start = 0;
	
	for (size_t i = 0; i < strlen(text) + 1; i++) {
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

// Convert the file_t datastructure back to a string for saving
// the function assumes that passed buffer is large enough
size_t code_to_string(file_t *code, char *buffer) {
	size_t offset = 0;

	for (int i = 0; i < code->line_amount; i++) {
		size_t line_len = strlen(code->lines[i].text);
		
		memcpy(buffer + offset, code->lines[i].text, (line_len + 1) * sizeof(char));
		
		if (i < code->line_amount - 1) {
			buffer[offset + line_len] = '\n';
		} else {
			buffer[offset + line_len] = '\0';
		}
		offset += line_len + 1;
	}

	return offset;
}

// Get size of code string
size_t code_get_len(file_t *code) {
	size_t len = 0;

	for (size_t i = 0; i < code->line_amount; i++) {
		len += strlen(code->lines[i].text);
	}

	return len;
}

// Split the line at the given position in 2
// a new line will be created with anything on the current line after the given pos
void split_line_at(file_t *code, int line, int pos, int indent_level) {
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
int merge_line(file_t *code, int line) {
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
void insert_char_at(file_t *code, int line, int pos, char c) {
	size_t len = strlen(code->lines[line].text);

	// + 2, 1 for null terminator and 1 for the new character
	code->lines[line].text = realloc(code->lines[line].text, (size_t)len + 2ULL);
	memmove(&code->lines[line].text[pos] + 1, &code->lines[line].text[pos], strlen(&code->lines[line].text[pos]) + 1ULL);

	code->lines[line].text[pos] = c;
}

// Remove a char at a position
void remove_char_at(file_t *code, int line, int pos) {
	if (pos == 0) {
		return;
	}

	// TODO: use this
	// line_t *line = &code->lines[line];

	size_t len = strlen(code->lines[line].text);

	memmove(&code->lines[line].text[pos] - 1, &code->lines[line].text[pos], strlen(&code->lines[line].text[pos]) + 1ULL);
	code->lines[line].text = realloc(code->lines[line].text, len);

	code->cursor_pos--;
}

// Wrapper
void insert_char_at_cursor(file_t *code, char c) {
	insert_char_at(code, code->cursor_line, code->cursor_pos, c);
	code->cursor_pos++;
}

int get_indent_level(char text[]) {
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