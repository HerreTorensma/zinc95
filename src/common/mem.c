#include "mem.h"

#include <inttypes.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>

static arena_t _temp_arena = {0};

// The asserts are commented out because it breaks the text_file functionality
// I will get back to that
void *alloc(allocator_t allocator, size_t size) {
	// assert(size > 0 && "You requested to allocate 0 bytes which is illegal");
	return allocator.proc(size, NULL, allocator.data, ALLOCATOR_ALLOCATE);
}

void dealloc(allocator_t allocator, void *data) {
	if (data != NULL) {
		allocator.proc(0, data, allocator.data, ALLOCATOR_DEALLOCATE);
	}
}

void *heap_alloc(size_t size) {
	assert(size > 0 && "You requested to allocate 0 bytes which is illegal");
	void *ptr = malloc(size);
	assert(ptr != NULL && "Malloc failed");
	memset(ptr, 0, size);
	return ptr;
}

void *heap_realloc(void *data, size_t new_size) {
	assert(new_size > 0 && "You requested to reallocate to 0 bytes which is illegal");
	void *ptr = realloc(data, new_size);
	assert(ptr != NULL && "Realloc failed");
	return ptr;
}

void heap_dealloc(void *data) {
	if (data != NULL) {
		free(data);
	}
}

void arena_init(arena_t *arena, size_t capacity) {
	arena->pos = 0;
	arena->capacity = capacity;
	
	arena->data = malloc(capacity);
	if (arena->data == NULL) {
		fprintf(stderr, "Failed to allocate memory for arena.\n");
	}
}

void *arena_alloc(arena_t *arena, size_t size) {
	assert(arena->pos < arena->capacity && "The temp memory is full");

	void *ptr = (uint8_t *)arena->data + arena->pos;
	memset(ptr, 0, size);

	arena->pos += size;

	return ptr;
}

void arena_clear(arena_t *arena) {
	arena->pos = 0;
}

void arena_free(arena_t *arena) {
	free(arena->data);
	arena->capacity = 0;
	arena->pos = 0;
}

void temp_mem_init(size_t capacity) {
	arena_init(&_temp_arena, capacity);
}

void *temp_alloc(size_t size) {
	return arena_alloc(&_temp_arena, size);
}

void temp_clear() {
	arena_clear(&_temp_arena);
}

void temp_free() {
	arena_free(&_temp_arena);
}

void *heap_allocator_proc(size_t size, void *existing, void *data, allocator_message_t message) {
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

void *arena_allocator_proc(size_t size, void *existing, void *data, allocator_message_t message) {
	switch (message) {
		case ALLOCATOR_ALLOCATE: {
			return arena_alloc(existing, size);
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

void *temp_allocator_proc(size_t size, void *existing, void *data, allocator_message_t message) {
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

allocator_t get_arena_allocator(arena_t *arena) {
	return (allocator_t){
		.proc = arena_allocator_proc,
		.data = (void*)arena,
	};
}

// TODO: make this
// allocator_t get_static_allocator(uint8_t *memory, size_t size) {
// 	return (allocator_t){
// 		.proc = 
// 	}
// }

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

	dealloc(allocator, *data);

	*data = new_data;
}