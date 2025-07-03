#pragma once

#include <stdlib.h>
#include <stdbool.h>

// Plans for length based strings:
// I'm going to use this everywhere related to the code editor and the saving/loading of the complete cartridge
// For the GUI functions I should make them accept a len_string, then
// also have a version that accepts a C null-terminated string but just calls strlen and then calls the len_string version
// For ease of use

// Length based string struct, not null-terminated
typedef struct {
	char *data;
	size_t len;
} string_t;

// Compile time evaluated string macro (not used obv)
// #define STR(s) {.data = s, .len = sizeof(s) - 1}

// Runtime evaluated string macro
#define STR(s) (string_t){.data = (char *)s, .len = strlen(s)}

// Some kind of array thing
typedef struct string_array {
	string_t *data;
	size_t size;
} string_array_t;

void string_array_push(string_array_t *array, string_t string);

string_t temp_alloc_string(size_t size);

// String functions
// any that use the temporary allocator have temp in the name somewhere
bool string_eq(string_t a, string_t b);

string_t string_concat_temp(string_t a, string_t b);

char *string_to_temp_c_string(string_t string);

string_t string_duplicate_temp(string_t string);

void print_string(string_t string);

void string_concat(string_t *dest, string_t src);

string_array_t string_split_to_temp(string_t string, char seperator);

// TODO: string_view function that returns a new string referencing part of anothers memory
// and use it in the tokenizer in the code editor