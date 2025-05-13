/*
String and text editor file datastructure and functions
*/
#pragma once

// #include "../computer.h"
#include <stdlib.h>

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

// #define STR(_string) ((len_string_t){.data = _string, .len = strlen(_string)})

typedef struct line {
	char *text;
	// len_string_t string;

	// Here will also be the token list,
	// that's why a line isn't just a plain string
} line_t;

// Datastructure to represent a text file in the text editor
// though they get compiled into one string
// It's just for organizational purposes 
typedef struct file {
	line_t *lines;
	size_t line_amount;
	
	int cursor_line;
	int cursor_pos;
	int target_pos;
} file_t;

// TODO: use either buffer or text, but keep it consistent

// Get the amount of lines in a string, used for loading
size_t string_get_lines_amount(const char *text);

// Get the indent level of the given string
int string_get_indent_level(char text[]);

// Initializes the given file with the given buffer
// Takes care of all allocation, so you can use this function at any time
// without worrying about memory leaks (at least that is the idea)
void file_load(file_t *file, const char *buffer);

// Add a new line to the data structure, used for loading a string before editing
// void file_add_line(file_t *file, const char *text, size_t len);

// Load a string into the file_t datastructure
// void string_to_file(file_t *file, const char *buffer);

// Gets the size of the string the data structure represents
// Adds +1 for the null terminator
size_t file_get_string_len(file_t *file);

// Convert the file_t datastructure back to a string for saving
// the function assumes that passed buffer is large enough
size_t file_to_string(file_t *file, char *buffer);

// Split the line at the given position in 2
// a new line will be created with anything on the current line after the given pos
void file_split_line_at(file_t *file, int line, int pos, int indent_level);

// Merge the given line with the line above it
int file_merge_line(file_t *file, int line);

// Insert a char at a position
void file_insert_char_at(file_t *file, int line, int pos, char c);

// Remove a char at a position
void file_remove_char_at(file_t *file, int line, int pos);

// Frees the file
void file_free(file_t *file);

// Higher level functions

// Inserts a character at the cursor
void file_insert_char_at_cursor(file_t *file, char c);

// These are self descriptory or something
// yk what I mean
void file_move_cursor_up(file_t *file);

void file_move_cursor_down(file_t *file);

void file_move_cursor_left(file_t *file);

void file_move_cursor_right(file_t *file);

void file_move_cursor_to_next_word(file_t *file);

void file_move_cursor_to_prev_word(file_t *file);