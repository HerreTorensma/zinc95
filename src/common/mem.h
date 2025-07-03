/*
Memory and string related functionality
A kind of more extensive version of string.h with added datastructures
*/

#pragma once

#include <stdlib.h>
#include <stdbool.h>

void temp_mem_init(size_t capacity);

void *temp_alloc(size_t size);

void *temp_calloc(size_t size);

void temp_clear();

void temp_free();

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