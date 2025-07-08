/*
Memory and string related functionality
A kind of more extensive version of string.h with added datastructures
*/

#pragma once

#define KB(x) (x * 1024ULL)
#define MB(x) ((KB(x)) * 1024ULL)

#include <stdlib.h>
#include <stdbool.h>

typedef enum allocator_message {
	ALLOCATOR_ALLOCATE,
	ALLOCATOR_REALLOCATE,
	ALLOCATOR_DEALLOCATE,
} allocator_message_t;

// The whole allocator stuff might be kind of overengineered and a simple enum value could suffice but this is more future proof
// And I like this way of doing things better I think
typedef struct allocator {
	void *(*proc)(size_t size, void *existing, allocator_message_t message);
} allocator_t;

void *alloc(allocator_t allocator, size_t size);

void dealloc(allocator_t allocator, void *data);

void *heap_alloc(size_t size);

void *heap_realloc(void *data, size_t new_size);

void heap_dealloc(void *data);

void temp_mem_init(size_t capacity);

void *temp_alloc(size_t size);

void temp_clear();

void temp_free();

void *heap_allocator_proc(size_t size, void *existing, allocator_message_t message);

void *temp_allocator_proc(size_t size, void *existing, allocator_message_t message);

allocator_t get_heap_allocator();

allocator_t get_temp_allocator();

// Non-growing stack datastructure
// I have to name it zinc_stack because struct stack and stack_t conflict with some MacOS stuff
// TODO: consider removing this and just using an array
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

// Array data structure
typedef struct array {
	void *data;
	size_t capacity;
	size_t item_size;
	size_t len;
	allocator_t allocator;
} array_t;

// Init the array with more options, useful for arrays not meant to be resized for example
void array_init_ex(array_t *array, size_t item_size, size_t capacity);

// Init the array
void array_init(array_t *array, size_t item_size);

// Append to the array
void array_push(array_t *array, void *item);

// Pop the array
void array_pop(array_t *array, void *item);

// Get the item at the given index, the item will be copied to the passed void *item
void array_get(array_t *array, size_t index, void *item);

// Set the item at the given index
void array_set(array_t *array, size_t index, void *item);

// Removes item at index and moves the items after it back
void array_remove_at(array_t *array, size_t index);

// Removes all items
void array_reset(array_t *array);