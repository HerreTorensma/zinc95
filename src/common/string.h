#pragma once

#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include "mem.h"

// Plans for length based strings:
// I'm going to use this everywhere related to the code editor and the saving/loading of the complete cartridge
// For the GUI functions I should make them accept a len_string, then
// also have a version that accepts a C null-terminated string but just calls strlen and then calls the len_string version
// For ease of use

// Length based string struct, not null-terminated
typedef struct string {
	uint64_t len;
	uint8_t *data;
} string_t;

// Compile time evaluated string macro (not used obv)
// #define STR(s) {.data = s, .len = sizeof(s) - 1}

// Runtime evaluated string macro
#define STR(s) (string_t){.data = (uint8_t *)s, .len = strlen(s)}

// Define string array
ARRAY_DEFINE(string_t)



string_t temp_alloc_string(size_t capacity);

// TODO: string dealloc functions

// String functions
// any that use the temporary allocator have temp in the name somewhere
bool string_eq(string_t a, string_t b);

string_t string_concat(allocator_t allocator, string_t a, string_t b);

char *string_to_c_string(allocator_t allocator, string_t string);

string_t string_copy(allocator_t allocator, string_t string);

void print_string(string_t string);

// void string_append(string_t *dest, string_t src);

string_t string_view(string_t source, size_t start, size_t len);

// Copies new_string to base, without doing any allocation
// Assumes base has enough memory allocated for new_string
// TODO: consider removing this function
void string_place(string_t *base, string_t new_string);

bool string_is_empty(string_t string);

// Returns an array of string views
string_t_array_t string_split(allocator_t allocator, string_t string, char seperator);



// --- String builder ---

typedef struct string_builder {
	string_t string;
	size_t capacity;
	allocator_t allocator;
	size_t position; // For reading
} string_builder_t;

// It grows automatically but the initial_capacity is still nice to prevent unnecessary allocations
void string_builder_init(string_builder_t *builder, allocator_t allocator, size_t initial_capacity);

void string_builder_append(string_builder_t *builder, string_t string);

void string_builder_append_raw(string_builder_t *builder, uint8_t *data, uint64_t len);

void string_builder_append_char(string_builder_t *builder, uint8_t c);

void string_builder_append_u8(string_builder_t *builder, uint8_t value);

void string_builder_append_u16(string_builder_t *builder, uint16_t value);

void string_builder_deinit(string_builder_t *builder);

bool is_alphabetic(char c);

bool is_digit(char c);

bool is_alphanumeric(char c);

bool is_whitespace(char c);

string_t int_to_string(allocator_t allocator, int number);

string_t int_to_string_formatted(allocator_t allocator, int number, int desired_length, char filler);

int string_to_int(string_t string);

// Appends a path to another path
string_t path_append(allocator_t allocator, string_t base, string_t appendage);

// Truncates the given path, returns a string view to the part of the path without the end directory or file
// Also removes the slash
string_t path_get_parent_dir(string_t path);

// Get the filename without the extension
string_t path_truncate_extension(string_t path);

// Get the filename without the extension
string_t path_get_filename(string_t path);

string_t path_get_filename_extension(string_t path);

size_t visual_string_pos_to_string_pos(string_t string, size_t pos, size_t tab_size);

size_t string_pos_to_visual_string_pos(string_t string, size_t pos, size_t tab_size);

uint8_t hex_char_to_value(char c);

int hex_string_to_binary(string_t hex_string, uint8_t *buffer, size_t size);

void bytes_to_hex(uint8_t bytes[], size_t len, char hex[]);

string_t format_string(allocator_t allocator, string_t base, ...);
