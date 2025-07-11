/*
Memory and string related functionality
A kind of more extensive version of string.h with added datastructures
*/

#pragma once

#define KB(x) (x * 1024ULL)
#define MB(x) ((KB(x)) * 1024ULL)

#include <stdlib.h>
#include <stdbool.h>



// --- Allocator ---

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



// --- Stack ---

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



// --- Array ---

// TODO: consider removing the automatic typedef so you don't get weird names like string_t_array_t 
#define ARRAY_DEFINE(type)  \
	typedef struct type##_array { \
		allocator_t allocator; \
		type *data; \
		size_t capacity; \
		size_t len; \
	} type##_array_t;

#ifndef ARRAY_INITIAL_CAPACITY
#define ARRAY_INITIAL_CAPACITY 8
#endif

size_t get_next_power_of_2(size_t number);

void _array_reserve(allocator_t allocator, void **data, size_t *capacity, size_t item_size, size_t needed_size);

// The following macros are in lowercase because it looks better
// and also they are more like actions instead of declarations or definitions so I think it's fair 

// array_init will also work on an already initialized array, but it will just reset it essentially
// but no memory leaks unless maybe you pass in a different allocator
// One catch is that the data pointer but be NULL otherwise it will attempt to free some random pointer found in uninitialized memory
#define array_init(array, _allocator)  \
	do { \
		(array)->allocator = (_allocator); \
		(array)->capacity = 0; \
		(array)->len = 0; \
		_array_reserve((array)->allocator, (void **)&(array)->data, &(array)->capacity, sizeof(*((array)->data)), ARRAY_INITIAL_CAPACITY); \
	} while (0);

#define array_append(array, item)  \
	do { \
		_array_reserve((array)->allocator, (void **)&(array)->data, &(array)->capacity, sizeof(*((array)->data)), (array)->len + 1); \
		(array)->data[(array)->len] = item; \
		(array)->len++; \
	} while (0);

// Clears the array, making it just like array_init except the allocator doesn't change
#define array_clear(array) \
	do { \
		(array)->len = 0; \
		_array_reserve((array)->allocator, (void **)&(array)->data, &(array)->capacity, sizeof(*((array)->data)), ARRAY_INITIAL_CAPACITY); \
	} while (0);

#define array_deinit(array) \
	do { \
		dealloc((array)->allocator, (array)->data); \
		(array)->data = NULL; \
		(array)->capacity = 0; \
		(array)->len = 0; \
	} while (0);
