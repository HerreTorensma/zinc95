/*
Unit tests for low level stuff
*/

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#include "common/mem.h"
#include "common/string.h"
#include "common/io.h"

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

	// string_t_array_t split_strings = string_split(get_heap_allocator(), STR("hello\nworld\n\n"), '\n');
	string_t_array_t split_strings = string_split(get_heap_allocator(), STR("hello\n\nworld\n\n"), '\n');
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

	int number = 345;
	string_t number_as_string = int_to_string(get_heap_allocator(), number);
	assert(string_eq(number_as_string, STR("345")) && "int_to_string failed");
	int number_as_number_again = string_to_int(number_as_string);
	assert(number_as_number_again == 345 && "string_to_int failed");

	assert(string_to_int(STR("0")) == 0 && "string_to_int failed");
	assert(string_to_int(STR("1")) == 1 && "string_to_int failed");
	
	// Path append
	assert(string_eq(path_append(get_heap_allocator(), STR("/"), STR("sample")), STR("/sample")) && "path_append failed");
	assert(string_eq(path_append(get_heap_allocator(), STR("/sample/"), STR("/sample2")), STR("/sample/sample2")) && "path_append failed");
	assert(string_eq(path_append(get_heap_allocator(), STR("what"), STR("the")), STR("what/the")) && "path_append failed");
	assert(string_eq(path_append(get_heap_allocator(), STR("yes/"), STR("no")), STR("yes/no")) && "path_append failed");
	assert(string_eq(path_append(get_heap_allocator(), STR("no"), STR("/yes")), STR("no/yes")) && "path_append failed");
	assert(string_eq(path_append(get_heap_allocator(), STR("maybe/"), STR("/idk")), STR("maybe/idk")) && "path_append failed");

	// Path get parent dir
	assert(string_eq(path_get_parent_dir(STR("/")), STR("/")) && "path_get_parent_dir failed");
	assert(string_eq(path_get_parent_dir(STR("/idk")), STR("/")) && "path_get_parent_dir failed");
	assert(string_eq(path_get_parent_dir(STR("/idk/ad")), STR("/idk")) && "path_get_parent_dir failed");
	assert(string_eq(path_get_parent_dir(STR("/idk/ad/what.txt")), STR("/idk/ad")) && "path_get_parent_dir failed");
	assert(string_eq(path_get_parent_dir(STR("idk")), STR("idk")) && "path_get_parent_dir failed");

	// path_truncate_extension
	assert(string_eq(path_truncate_extension(STR("thing.txt")), STR("thing")) && "path_get_filename failed");
	assert(string_eq(path_truncate_extension(STR("thing")), STR("thing")) && "path_get_filename failed");

	// path_get_filename
	assert(string_eq(path_get_filename(STR("")), STR("")) && "path_get_filename failed");
	assert(string_eq(path_get_filename(STR("/")), STR("")) && "path_get_filename failed");
	assert(string_eq(path_get_filename(STR("/thing")), STR("thing")) && "path_get_filename failed");
	assert(string_eq(path_get_filename(STR("thing")), STR("thing")) && "path_get_filename failed");
	assert(string_eq(path_get_filename(STR("some/path/thing")), STR("thing")) && "path_get_filename failed");
	assert(string_eq(path_get_filename(STR("some/path/thing.txt")), STR("thing.txt")) && "path_get_filename failed");
	assert(string_eq(path_get_filename(STR("some/path/thing.txt")), STR("thing.txt")) && "path_get_filename failed");
	assert(string_eq(path_get_filename(STR("/some/path/thing.txt")), STR("thing.txt")) && "path_get_filename failed");
}

void run_tests(void) {
	printf("Testing array...\n");
	_test_array();
	printf("OK!\n\n");

	printf("Testing string...\n");
	_test_string();
	printf("OK!\n\n");
}