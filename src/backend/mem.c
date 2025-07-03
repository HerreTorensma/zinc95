#include "mem.h"

#include <inttypes.h>
#include <stdio.h>
#include <string.h>

static arena_t _temp_arena = {0};

void arena_init(arena_t *arena, size_t initial_size) {
	arena->pos = 0;
	arena->size = initial_size;
	
	arena->data = malloc(initial_size);
	if (arena->data == NULL) {
		printf("Failed to allocate memory for arena.\n");
	}
}

void *arena_alloc(arena_t *arena, size_t size) {
	if (arena->pos >= arena->size) {
		printf("The arena is full.\n");
		return;
	}

	void *ptr = (uint8_t *)arena->data + arena->pos;

	arena->pos += size;

	return ptr;
}

void *arena_calloc(arena_t *arena, size_t size) {
	void *ptr = arena_alloc(arena, size);
	memset(ptr, 0, size);

	return ptr;
}

void arena_clear(arena_t *arena) {
	arena->pos = 0;
}

void arena_free(arena_t *arena) {
	free(arena->data);
}

void temp_arena_init(size_t initial_size) {
	arena_init(&_temp_arena, initial_size);
}

void *temp_alloc(size_t size) {
	return arena_alloc(&_temp_arena, size);
}

void *temp_calloc(size_t size) {
	return arena_calloc(&_temp_arena, size);
}

void temp_clear() {
	arena_clear(&_temp_arena);
}

void temp_free() {
	arena_free(&_temp_arena);
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

void string_array_push(string_array_t *array, string_t string) {
	array->data[array->size] = string;
	array->size++;
}

string_t temp_alloc_string(size_t size) {
	return (string_t) {
		.data = temp_alloc(size * sizeof(char)),
		// TODO: why is the len 0? there was a reason for it but i don't remember
		.len = 0,
	};
}

bool string_eq(string_t a, string_t b) {
	if (a.len != b.len) {
		return false;
	}

	for (size_t i = 0; i < a.len; i++) {
		if (a.data[i] != b.data[i]) {
			return false;
		}
	}

	return true;
}

string_t string_concat_temp(string_t a, string_t b) {
	string_t new_string = {
		.data = temp_alloc((a.len + b.len) * sizeof(char)),
		.len = a.len + b.len,
	};

	memcpy(new_string.data, a.data, a.len * sizeof(char));
	memcpy(new_string.data + a.len, b.data, b.len * sizeof(char));

	return new_string;
}

char *string_to_temp_c_string(string_t string) {
	char *buffer = temp_alloc((string.len + 1) * sizeof(char));
	memcpy(buffer, string.data, string.len);
	buffer[string.len] = '\0';

	return buffer;
}

string_t string_duplicate_temp(string_t string) {
	string_t new_string = temp_alloc_string(string.len);
	memcpy(new_string.data, string.data, string.len * sizeof(char));
	return new_string;
}

void print_string(string_t string) {
	for (size_t i = 0; i < string.len; i++) {
		putchar(string.data[i]);
	}
}

void string_concat(string_t *dest, string_t src) {
	size_t old_len = dest->len;
	dest->len += src.len;

	memcpy(dest->data + old_len, src.data, src.len);
}

string_array_t string_split_to_temp(string_t string, char seperator) {
	size_t items_amount = 0;
	
	for (size_t i = 0; i < string.len; i++) {
		if (string.data[i] == seperator) {
			items_amount++;
		}
	}
	items_amount++;
	
	string_array_t array = {
		.data = temp_alloc(items_amount * sizeof(string_t)),
		.size = 0,
	};

	size_t last_index = 0;
	for (size_t i = 0; i < string.len; i++) {
		if (string.data[i] == seperator) {
			string_t substring = {
				.data = string.data + last_index,
				.len = i - last_index,
			};
			string_array_push(&array, substring);

			last_index = i + 1;
		}
	}

	if (last_index < string.len) {
		string_t substring = {
			.data = string.data + last_index,
			.len = string.len - last_index,
		};
		string_array_push(&array, substring);
	}

	return array;
}