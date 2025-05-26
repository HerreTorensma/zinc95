#pragma once

#include <stdlib.h>
#include <stdbool.h>

// Non-growing stack datastructure
typedef struct stack {
	void *items;
	size_t capacity;
	size_t item_size;
	size_t len;
} stack_t;

void stack_init(stack_t *stack, size_t item_size, size_t capacity);

void stack_push(stack_t *stack, void *item);

bool stack_pop(stack_t *stack, void *item);

void stack_quit(stack_t *stack);