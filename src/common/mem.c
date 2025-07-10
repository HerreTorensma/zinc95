#include "mem.h"

#include <inttypes.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>

typedef struct temp_mem {
	void *data;
	size_t capacity;
	size_t pos;
} temp_mem_t;

static temp_mem_t _temp_mem = {0};

void *alloc(allocator_t allocator, size_t size) {
	assert(size > 0 && "You requested to allocate 0 bytes which is illegal");
	return allocator.proc(size, NULL, ALLOCATOR_ALLOCATE);
}

void dealloc(allocator_t allocator, void *data) {
	assert(data != NULL && "You tried to deallocate a NULL pointer");
	allocator.proc(0, data, ALLOCATOR_DEALLOCATE);
}

void *heap_alloc(size_t size) {
	assert(size > 0 && "You requested to allocate 0 bytes which is illegal");
	void *ptr = malloc(size);
	memset(ptr, 0, size);
	return ptr;
}

void *heap_realloc(void *data, size_t new_size) {
	assert(new_size > 0 && "You requested to allocate 0 bytes which is illegal");
	void *ptr = realloc(data, new_size);
	return ptr;
}

void heap_dealloc(void *data) {
	assert(data != NULL && "You tried to deallocate a NULL pointer");
	free(data);
}

void temp_mem_init(size_t capacity) {
	_temp_mem.pos = 0;
	_temp_mem.capacity = capacity;
	
	_temp_mem.data = malloc(capacity);
	if (_temp_mem.data == NULL) {
		fprintf(stderr, "Failed to allocate memory for arena.\n");
	}
}

void *temp_alloc(size_t size) {
	assert(_temp_mem.pos < _temp_mem.capacity && "The temp memory is full");

	void *ptr = (uint8_t *)_temp_mem.data + _temp_mem.pos;
	memset(ptr, 0, size);

	_temp_mem.pos += size;

	return ptr;
}

void temp_clear() {
	_temp_mem.pos = 0;
}

void temp_free() {
	free(_temp_mem.data);
	_temp_mem.capacity = 0;
	_temp_mem.pos = 0;
}

void *heap_allocator_proc(size_t size, void *existing, allocator_message_t message) {
	switch (message) {
		case ALLOCATOR_ALLOCATE: {
			return heap_alloc(size);
		}
		case ALLOCATOR_REALLOCATE: {
			return heap_realloc(existing, size);
		}
		case ALLOCATOR_DEALLOCATE: {
			heap_dealloc(existing);
			return NULL;
		}
	}

	return NULL;
}

void *temp_allocator_proc(size_t size, void *existing, allocator_message_t message) {
	switch (message) {
		case ALLOCATOR_ALLOCATE: {
			return temp_alloc(size);
		}
		case ALLOCATOR_REALLOCATE: {
			printf("Temporary allocator cannot reallocate");
			return NULL;
		}
		case ALLOCATOR_DEALLOCATE: {
			// You can't free temporary memory
			return NULL;
		}
	}

	return NULL;
}

allocator_t get_heap_allocator() {
	return (allocator_t){
		.proc = heap_allocator_proc,
	};
}

allocator_t get_temp_allocator() {
	return (allocator_t){
		.proc = temp_allocator_proc,
	};
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

size_t get_next_power_of_2(size_t number) {
	if (number == 0) {
		return 1;
	}

	number--;
	number |= number >> 1;
	number |= number >> 2;
	number |= number >> 4;
	number |= number >> 8;
	number |= number >> 16;
	number |= number >> 32;

	return number + 1;
}

void _array_reserve(allocator_t allocator, void **data, size_t *capacity, size_t item_size, size_t needed_size) {
	if (*capacity >= needed_size) {
		return;
	}

	size_t old_capacity = *capacity;

	*capacity = get_next_power_of_2(needed_size);

	void *new_data = alloc(allocator, *capacity * item_size);
	memcpy(new_data, *data, old_capacity * item_size);

	if (*data != NULL) {
		dealloc(allocator, *data);
	}

	*data = new_data;
}