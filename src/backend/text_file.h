/*
Text editor file datastructure and functions
*/
#pragma once

#include "../computer.h"

// Plans for length based strings:
// I'm going to use this everywhere related to the code editor and the saving/loading of the complete cartridge
// For the GUI functions I should make them accept a len_string, then
// also have a version that accepts a C null-terminated string but just calls strlen and then calls the len_string version
// For ease of use

// Length based string struct
typedef struct len_string {
	char *data;
	size_t len;
} len_string_t;

#define STR(_string) ((len_string_t){.data = _string, .len = strlen(_string)})

// Get the amount of lines in a string, used for loading
size_t string_get_lines_amount(const char *text);

// Append a new line, used for loading a string before editing
void line_append(file_t *code, const char *text, size_t len);

// Load a string into the file_t datastructure
void string_to_code(file_t *code, char *text);

// Convert the file_t datastructure back to a string for saving
// the function assumes that passed buffer is large enough
size_t code_to_string(file_t *code, char *buffer);

// Get size of code string
size_t code_get_len(file_t *code);

// Split the line at the given position in 2
// a new line will be created with anything on the current line after the given pos
void split_line_at(file_t *code, int line, int pos, int indent_level);

// Merge the given line with the line above it
int merge_line(file_t *code, int line);

// Insert a char at a position
void insert_char_at(file_t *code, int line, int pos, char c);

// Remove a char at a position
void remove_char_at(file_t *code, int line, int pos);

void insert_char_at_cursor(file_t *code, char c);

// Get the indent level of the given string
int get_indent_level(char text[]);