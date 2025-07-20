/*
String and text editor file datastructure and functions
*/
#pragma once

#include "../common/string.h"
#include <stdlib.h>

typedef enum lua_token_type {
	LUA_TOKEN_KEYWORD,
	LUA_TOKEN_BUILTIN_FUNCTION,
	LUA_TOKEN_IDENTIFIER,
	LUA_TOKEN_LITERAL, // Number, true, false, nil
	LUA_TOKEN_STRING,
	LUA_TOKEN_COMMENT,
	LUA_TOKEN_OPERATOR,
	LUA_TOKEN_WHITESPACE,

	LUA_TOKEN_COUNT,
} lua_token_type_t;

typedef struct lua_token {
	string_t string;
	lua_token_type_t type;
} lua_token_t;

ARRAY_DEFINE(lua_token_t)

typedef struct line {
	string_t string;
	lua_token_t_array_t tokens;
} line_t;

// Datastructure to represent a text file in the text editor
// though they get compiled into one string
// It's just for organizational purposes 
typedef struct file {
	line_t *lines;
	size_t line_amount;
	
	size_t cursor_line;
	size_t cursor_pos;
	size_t target_pos; // TODO: I don't think this belongs on the data structure, should be moved to the editor
} file_t;

// Get the indent level of the given string
// TODO: make this string_t
int string_get_indent_level(const char text[]);

void file_append_line(file_t *file, string_t line_view);

string_t file_to_string(file_t *file, allocator_t allocator);

// Split the line at the given position in 2
// a new line will be created with anything on the current line after the given pos
void file_split_line_down(file_t *file, size_t line, size_t pos, size_t indent_level);

// Merge the given line with the line above it
size_t file_merge_line_up(file_t *file, size_t line);

// Insert a char at a position
void file_insert_char_at(file_t *file, size_t line, size_t pos, char c);

// Remove a char at a position
void file_remove_char_at(file_t *file, size_t line, size_t pos);

// Deinitializes the file
void file_deinit(file_t *file);



// --- Higher level functions ---

// Inserts a character at the cursor
void file_insert_char_at_cursor(file_t *file, char c);

// Removes the character at the cursor and merges with the above line if necessary cursor pos is 0
void file_remove_char_at_cursor(file_t *file);

// These are self descriptory or something
// yk what I mean
void file_move_cursor_up(file_t *file);

void file_move_cursor_down(file_t *file);

void file_move_cursor_left(file_t *file);

void file_move_cursor_right(file_t *file);

void file_move_cursor_to_next_word(file_t *file);

void file_move_cursor_to_prev_word(file_t *file);