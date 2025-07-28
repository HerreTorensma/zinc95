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
	char *data;
	size_t len;
} string_t;

// Compile time evaluated string macro (not used obv)
// #define STR(s) {.data = s, .len = sizeof(s) - 1}

// Runtime evaluated string macro
#define STR(s) (string_t){.data = (char *)s, .len = strlen(s)}

// Define string array
ARRAY_DEFINE(string_t)



string_t temp_alloc_string(size_t capacity);

// String functions
// any that use the temporary allocator have temp in the name somewhere
bool string_eq(string_t a, string_t b);

string_t string_concat(allocator_t allocator, string_t a, string_t b);

char *string_to_c_string(allocator_t allocator, string_t string);

string_t string_copy(allocator_t allocator, string_t string);

void print_string(string_t string);

// void string_append(string_t *dest, string_t src);

string_t string_view(string_t source, size_t start, size_t len);

string_t_array_t string_split(allocator_t allocator, string_t string, char seperator);



// --- String builder ---

typedef struct string_builder {
	string_t string;
	size_t capacity;
	allocator_t allocator;
} string_builder_t;

// It grows automatically but the initial_capacity is still nice to prevent unnecessary allocations
void string_builder_init(string_builder_t *builder, allocator_t allocator, size_t initial_capacity);

void string_builder_append(string_builder_t *builder, string_t string);

void string_builder_deinit(string_builder_t *builder);

bool is_alphabetic(char c);

bool is_digit(char c);

bool is_alphanumeric(char c);

bool is_whitespace(char c);

string_t int_to_string(allocator_t allocator, int number);

int string_to_int(string_t string);
