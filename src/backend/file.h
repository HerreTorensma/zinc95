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

typedef struct string_reference {
	size_t start;
	size_t len;
} string_reference_t;

ARRAY_DEFINE(string_reference_t)

typedef struct lua_token {
	string_reference_t string_reference;
	lua_token_type_t type;
} lua_token_t;

ARRAY_DEFINE(lua_token_t)

typedef enum file_action_type {
	FILE_ACTION_INVALID,
	FILE_ACTION_INSERT,
	FILE_ACTION_REMOVE,
	FILE_ACTION_MOVE,
} file_action_type_t;

typedef struct file_action {
	size_t pos;
	string_t string;
	bool was_selection;

	file_action_type_t type;
} file_action_t;

ARRAY_DEFINE(file_action_t)

typedef struct file {
	string_t string;
	size_t size;
	size_t capacity;

	struct {
		int cursor_pos;
		int selection_start;
		int selection_end;
		int scroll_amount; // Scroll amount in lines
		string_reference_t_array_t lines;
		lua_token_t_array_t tokens;
		bool supress_mouse_selection; // Set to true when the selection was just removed, to prevent from instantly making a new selection

		file_action_t_array_t history;
		bool has_pending_insert;
		size_t pending_insert_start;
		bool has_pending_remove;
		size_t pending_remove_start;
	} edit_state;
} file_t;

size_t file_get_line_index_from_pos(file_t *file, size_t pos);

void file_insert_string_at(file_t *file, int pos, string_t string);

void file_insert_char_at(file_t *file, int pos, char c);

void file_append_string(file_t *file, string_t string);

void file_remove_section(file_t *file, int pos, size_t size);

void file_clear(file_t *file);

// For any given position, get the line 'around' it as a string with the \n afterwards included
// string_t get_line_string_view_around_pos(file_t *file, size_t pos);

size_t get_token_index_around_pos(file_t *file, int pos);

// size_t file_move_pos_up(file_t *file, size_t pos);
// // Get the index at the same offset from the beginning of the line pos is on, but the line above
// size_t file_move_pos_down(file_t *file, size_t pos);

size_t file_move_pos_vertical(file_t *file, int pos, int64_t amount);

bool file_does_selection_exist(file_t *file);

// Returns a string view to the first line of the file with -- and whitespace at the beginning trimmed off
string_t file_get_name(file_t *file);

// Swaps the selection_start and selection_end if necessary
void file_fix_selection(file_t *file);

void file_deselect(file_t *file);
