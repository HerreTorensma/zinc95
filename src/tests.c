/*
Unit tests for low level stuff
*/

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#include "common/mem.h"
#include "common/string.h"

static void _test_array(void) {
	ARRAY_DEFINE(int)

	int_array_t array = {0};
	array_init(&array, get_heap_allocator());

	// printf("array capacity: %d\n", array.capacity);
	assert(array.capacity == 8 && "Array does not have the right capacity of 8");
	
	array_append(&array, 4);
	array_append(&array, 6);
	array_append(&array, 12);
	array_append(&array, 12);
	array_append(&array, 12);
	array_append(&array, 12);
	array_append(&array, 12);
	array_append(&array, 12);
	array_append(&array, 12);
	
	// printf("after array capacity: %d\n", array.capacity);
	assert(array.capacity == 16 && "Array does not have the right capacity of 16");

	for (size_t i = 0; i < array.len; i++) {
		printf("array: %d\n", array.data[i]);
	}

	array_clear(&array);

	assert(array.len == 0 && "Array should have been cleared but wasn't");

	array_deinit(&array);
	assert(array.data == NULL && "Array memory was not cleared after deinitialization");
}

static void _test_string(void) {
	assert(string_eq(STR("test"), STR("test")) == true && "The string_eq function is completely broken");
	assert(string_eq(STR("test"), STR("test1")) == false && "The string_eq function is completely broken");

	string_t hello = STR("hello");
	string_t world = STR("world");

	string_t helloworld = string_concat(get_heap_allocator(), hello, world);
	assert(string_eq(helloworld, STR("helloworld")));

	string_t_array_t split_strings = string_split(get_heap_allocator(), STR("hello\nworld\n\n"), '\n');
	for (size_t i = 0; i < split_strings.len; i++) {
		printf("'");
		print_string(split_strings.data[i]);
		printf("', ");
	}

	printf("now strtok\n");
	
	char other_string[] = "hello\nworld\n\n";
	char *token = strtok(other_string, "\n");
	while (token != NULL) {
		printf("'%s', ", token);
		token = strtok(NULL, "\n");
	}
}

void run_tests(void) {
	printf("Testing array...\n");
	_test_array();
	printf("OK!\n\n");

	printf("Testing string...\n");
	_test_string();
	printf("OK!\n\n");
}