#pragma once

#include <stdlib.h>
#include <stdbool.h>

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
