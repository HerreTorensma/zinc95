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

bool string_eq(string_t a, string_t b);