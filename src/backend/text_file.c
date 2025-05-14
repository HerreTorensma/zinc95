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
static void file_add_line(file_t *file, const char *text, size_t len) {
	file->lines[file->line_amount].text = malloc((len + 1) * sizeof(char));
	if (file->lines[file->line_amount].text == NULL) {
		printf("Couldn't allocate memory for new line\n");
		exit(1);
	}

	memcpy(file->lines[file->line_amount].text, text, len);

	file->lines[file->line_amount].text[len] = '\0';
	file->line_amount++;
}

// Load a string into the file_t datastructure
static void string_to_file(file_t *file, const char *buffer) {
	char *last_line_start = buffer;
	size_t pos_since_last_line_start = 0;
	
	for (size_t i = 0; i < strlen(buffer) + 1; i++) {
		if (buffer[i] == '\0') {
			break;
		}

		if (buffer[i] == '\n') {
			if (last_line_start != buffer) {
				last_line_start++;
			}
			file_add_line(file, last_line_start, pos_since_last_line_start);

			pos_since_last_line_start = 0;
			last_line_start = &buffer[i];
			
			continue;
		}

		pos_since_last_line_start++;
	}
}

void file_load(file_t *file, const char *buffer) {
	// Apparantly calling free on a null pointer has no effect
	// so this is safe to do even if nothing is allocated yet
	file_free(file);

	// Allocate
	size_t lines_amount = string_get_lines_amount(buffer);
	file->lines = malloc(lines_amount * sizeof(line_t));
	if (file->lines == NULL) {
		printf("Couldn't allocate memory for code\n");
		exit(1);
	}

	string_to_file(file, buffer);
}

// TODO: actually use this
size_t file_get_string_len(file_t *file) {
	size_t len = 0;

	for (size_t i = 0; i < file->line_amount; i++) {
		len += strlen(file->lines[i].text);
		
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
		size_t line_len = strlen(file->lines[i].text);
		
		memcpy(buffer + offset, file->lines[i].text, (line_len + 1) * sizeof(char));
		
		if (i < file->line_amount - 1) {
			buffer[offset + line_len] = '\n';
		} else {
			buffer[offset + line_len] = '\0';
		}
		offset += line_len + 1;
	}

	return offset;
}

void file_split_line_at(file_t *file, int line, int pos, int indent_level) {
	// Realloc lines (not for now)
	file->lines = realloc(file->lines, ((size_t)file->line_amount + 1ULL) * sizeof(line_t));

	// Move the lines
	memmove(&file->lines[line + 1], &file->lines[line], ((size_t)file->line_amount - (size_t)line) * sizeof(line_t));
	file->line_amount++;

	char *after_cursor = &file->lines[line].text[pos];
	int after_cursor_len = strlen(after_cursor);

	file->lines[line + 1].text = malloc(((size_t)after_cursor_len + 1ULL) * sizeof(char));
	if (file->lines[line + 1].text == NULL) {
		printf("Couldn't allocate memory for new line\n");
		exit(1);
	}

	memcpy(file->lines[line + 1].text, after_cursor, (size_t)after_cursor_len + 1ULL);
	file->lines[line + 1].text[after_cursor_len] = '\0';

	file->lines[line].text[pos] = '\0';
	file->lines[line].text = realloc(file->lines[line].text, strlen(file->lines[line].text) + 1ULL);

	// TODO: Make this work
	// if (strlen(file->lines[line + 1].text) == 0) {
	// 	for (int i = 0; i < indent_level; i++) {
	// 		file->lines[line + 1].text[i] = '\t';
	// 	}
	// }
}

int file_merge_line(file_t *file, int line) {
	size_t old_line_len = strlen(file->lines[line - 1].text);
	size_t new_line_len = old_line_len + strlen(file->lines[line].text);

	file->lines[line - 1].text = realloc(file->lines[line - 1].text, ((size_t)new_line_len + 1ULL) * sizeof(char));
	strcat(file->lines[line - 1].text, file->lines[line].text);

	free(file->lines[line].text);
	file->lines[line].text = NULL;

	memmove(&file->lines[line], &file->lines[line + 1], ((size_t)file->line_amount - (size_t)line - 1ULL) * sizeof(line_t));
	
	file->line_amount--;

	return old_line_len;
}

void file_insert_char_at(file_t *file, int line, int pos, char c) {
	size_t len = strlen(file->lines[line].text);

	// + 2, 1 for null terminator and 1 for the new character
	file->lines[line].text = realloc(file->lines[line].text, (size_t)len + 2ULL);
	memmove(&file->lines[line].text[pos] + 1, &file->lines[line].text[pos], strlen(&file->lines[line].text[pos]) + 1ULL);

	file->lines[line].text[pos] = c;
}

void file_remove_char_at(file_t *file, int line, int pos) {
	if (pos == 0) {
		return;
	}

	// TODO: use this
	// line_t *line = &file->lines[line];

	size_t len = strlen(file->lines[line].text);

	memmove(&file->lines[line].text[pos] - 1, &file->lines[line].text[pos], strlen(&file->lines[line].text[pos]) + 1ULL);
	file->lines[line].text = realloc(file->lines[line].text, len);

	file->cursor_pos--;
}

void file_free(file_t *file) {
	for (size_t i = 0; i < file->line_amount; i++) {
		free(file->lines[i].text);
	}
	free(file->lines);
}

void file_insert_char_at_cursor(file_t *file, char c) {
	file_insert_char_at(file, file->cursor_line, file->cursor_pos, c);
	file->cursor_pos++;
}

void file_move_cursor_up(file_t *file) {
	if (file->cursor_line > 0) {
		file->cursor_line--;

		int len = strlen(file->lines[file->cursor_line].text);

		if (file->cursor_pos > len) {
			file->cursor_pos = len;
		}
	}
}

void file_move_cursor_down(file_t *file) {
	if (file->cursor_line < file->line_amount - 1) {
		file->cursor_line++;
	}

	int len = strlen(file->lines[file->cursor_line].text);
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
			file->cursor_pos = strlen(file->lines[file->cursor_line].text);
		}
	}
}

void file_move_cursor_right(file_t *file) {
	int len = strlen(file->lines[file->cursor_line].text);
	if (file->cursor_pos < len) {
		file->cursor_pos++;
	} else {
		if (file->cursor_line < file->line_amount - 1) {
			file->cursor_line++;
			file->cursor_pos = 0;
		}
	}
}

void file_move_cursor_to_next_word(file_t *file) {
	int len = strlen(file->lines[file->cursor_line].text);
			
	for (size_t i = file->cursor_pos + 1; i < len + 1; i++) {
		if (file->lines[file->cursor_line].text[i] == ' ' || file->lines[file->cursor_line].text[i] == '.' || i == len) {
			file->cursor_pos = i;
			break;
		}
	}
}

void file_move_cursor_to_prev_word(file_t *file) {
	for (size_t i = file->cursor_pos - 1; i >= 0; i--) {
		if (file->lines[file->cursor_line].text[i] == ' ' || file->lines[file->cursor_line].text[i] == '.' || i == 0) {
			file->cursor_pos = i;
			break;
		}
	}
}