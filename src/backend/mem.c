#include "mem.h"

#include <inttypes.h>

void stack_init(stack_t *stack, size_t item_size, size_t capacity) {
	stack->len = 0;
	stack->capacity = capacity;
	stack->item_size = item_size;

	stack->items = calloc(capacity, item_size);
}

void stack_push(stack_t *stack, void *item) {
	if (stack->len >= stack->capacity) {
		// Stack if full
		printf("Stack has reached capacity of %zu\n", stack->capacity);
		return;
	}

	memcpy((uint8_t *)stack->items + (stack->len * stack->item_size), item, stack->item_size);
	stack->len++;
}

// Returns false if stack is empty
bool stack_pop(stack_t *stack, void *item) {
	if (stack->len == 0) {
		printf("Stack is empty\n");
		// Stack is already empty
		// So memset the thing to zero I guess
		memset(item, 0, stack->item_size);
		return false;
	}
	
	stack->len--;

	memcpy(item, (uint8_t *)stack->items + (stack->len * stack->item_size), stack->item_size);

	return true;
}

void stack_quit(stack_t *stack) {
	free(stack->items);
	stack->items = NULL;
}