#include "mem.h"

#include <inttypes.h>
#include <stdio.h>
#include <string.h>

typedef struct temp_mem {
	void *data;
	size_t capacity;
	size_t pos;
} temp_mem_t;

static temp_mem_t _temp_mem = {0};

void temp_mem_init(size_t capacity) {
	_temp_mem.pos = 0;
	_temp_mem.capacity = capacity;
	
	_temp_mem.data = malloc(capacity);
	if (_temp_mem.data == NULL) {
		fprintf(stderr, "Failed to allocate memory for arena.\n");
	}
}

void *temp_alloc(size_t size) {
	if (_temp_mem.pos >= _temp_mem.capacity) {
		fprintf(stderr, "The temporary memory is full.\n");
		return NULL;
	}

	void *ptr = (uint8_t *)_temp_mem.data + _temp_mem.pos;

	_temp_mem.pos += size;

	return ptr;
}

void *temp_calloc(size_t size) {
	void *data = temp_alloc(size);
	memset(data, 0, size);

	return data;
}

void temp_clear() {
	_temp_mem.pos = 0;
}

void temp_free() {
	free(_temp_mem.data);
	_temp_mem.capacity = 0;
	_temp_mem.pos = 0;
}

void stack_init(zinc_stack_t *stack, size_t item_size, size_t capacity) {
	stack->len = 0;
	stack->capacity = capacity;
	stack->item_size = item_size;

	stack->items = calloc(capacity, item_size);
}

// TODO: when the stack is empty move the items down and still add the new one
void stack_push(zinc_stack_t *stack, void *item) {
	if (stack->len >= stack->capacity) {
		// Stack if full
		printf("Stack has reached capacity of %zu\n", stack->capacity);
		return;
	}

	memcpy((uint8_t *)stack->items + (stack->len * stack->item_size), item, stack->item_size);
	stack->len++;
}

// Returns false if stack is empty
bool stack_pop(zinc_stack_t *stack, void *item) {
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

void stack_quit(zinc_stack_t *stack) {
	free(stack->items);
	stack->items = NULL;
}
