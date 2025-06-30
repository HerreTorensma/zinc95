/*
Memory and string related functionality
A kind of more extensive version of string.h with added datastructures
*/

#pragma once

#include <stdlib.h>
#include <stdbool.h>

// Memory arena
typedef struct arena {
	void *data;
	size_t pos;
	size_t size;
} arena_t;

void arena_init(arena_t *arena, size_t initial_size);

void *arena_alloc(arena_t *arena, size_t size);

void *arena_calloc(arena_t *arena, size_t size);

void arena_clear(arena_t *arena);

void arena_free(arena_t *arena);

void temp_arena_init(size_t initial_size);

void *temp_alloc(size_t size);

void *temp_calloc(size_t size);

void temp_clear();

void temp_free();

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

// Non-growing stack datastructure
// I have to name it zinc_stack because struct stack and stack_t conflict with some MacOS stuff
typedef struct zinc_stack {
	void *items;
	size_t capacity;
	size_t item_size;
	size_t len;
} zinc_stack_t;

void stack_init(zinc_stack_t *stack, size_t item_size, size_t capacity);

void stack_push(zinc_stack_t *stack, void *item);

bool stack_pop(zinc_stack_t *stack, void *item);

void stack_quit(zinc_stack_t *stack);

// Some kind of array thing
typedef struct string_array {
	string_t *data;
	size_t size;
} string_array_t;

void string_array_push(string_array_t *array, string_t string);

// String functions
// any that use the temporary allocator have temp in the name somewhere
bool string_eq(string_t a, string_t b);

char *string_to_temp_c_string(string_t string);

string_array_t string_split_to_temp(string_t string, char seperator);