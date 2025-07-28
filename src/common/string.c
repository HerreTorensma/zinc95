#include "string.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>

string_t temp_alloc_string(size_t capacity) {
	return (string_t) {
		.data = temp_alloc(capacity * sizeof(char)),
		// The len is zero since you're allocating an empty string
		// although I might consider changing this? Idk if there is a good reason
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

string_t string_concat(allocator_t allocator, string_t a, string_t b) {
	string_t new_string = {
		.data = alloc(allocator, (a.len + b.len) * sizeof(char)),
		.len = a.len + b.len,
	};

	memcpy(new_string.data, a.data, a.len * sizeof(char));
	memcpy(new_string.data + a.len, b.data, b.len * sizeof(char));

	return new_string;
}

char *string_to_c_string(allocator_t allocator, string_t string) {
	char *buffer = alloc(allocator, (string.len + 1) * sizeof(char));
	memcpy(buffer, string.data, string.len);
	buffer[string.len] = '\0';

	return buffer;
}

string_t string_copy(allocator_t allocator, string_t string) {
	// assert(string.len > 0);

	string_t new_string = {
		.data = alloc(allocator, string.len * sizeof(char)),
		.len = string.len,
	};

	memcpy(new_string.data, string.data, string.len * sizeof(char));
	
	return new_string;
}

void print_string(string_t string) {
	for (size_t i = 0; i < string.len; i++) {
		putchar(string.data[i]);
	}
}

string_t string_view(string_t source, size_t start, size_t len) {
	return (string_t){
		.data = source.data + start,
		.len = len,
	};
}

// TODO: don't include strings that are exactly the seperator
string_t_array_t string_split(allocator_t allocator, string_t string, char seperator) {
	size_t items_amount = 0;
	
	for (size_t i = 0; i < string.len; i++) {
		if (string.data[i] == seperator) {
			items_amount++;
		}
	}
	items_amount++;
	
	string_t_array_t array = {0};
	array_init(&array, allocator);

	size_t last_index = 0;
	for (size_t i = 0; i < string.len; i++) {
		if (string.data[i] == seperator) {
			string_t substring = string_view(string, last_index, i - last_index);

			array_append(&array, substring);

			last_index = i + 1;
		}
	}

	if (last_index < string.len) {
		string_t substring = string_view(string, last_index, string.len - last_index);
		array_append(&array, substring);
	}

	return array;
}

static void _string_builder_reserve(string_builder_t *builder, size_t needed_capacity) {
	if (builder->capacity >= needed_capacity) {
		return;
	}

	size_t old_capacity = builder->capacity;

	builder->capacity = get_next_power_of_2(needed_capacity);

	char *new_data = alloc(builder->allocator, builder->capacity * sizeof(char));
	memcpy(new_data, builder->string.data, old_capacity * sizeof(char));

	if (builder->string.data != NULL) {
		dealloc(builder->allocator, builder->string.data);
	}

	builder->string.data = new_data;
}

void string_builder_init(string_builder_t *builder, allocator_t allocator, size_t initial_capacity) {
	builder->allocator = allocator;
	builder->capacity = 0;
	builder->string = (string_t){0};

	_string_builder_reserve(builder, initial_capacity);
}

void string_builder_append(string_builder_t *builder, string_t string) {
	_string_builder_reserve(builder, builder->string.len + string.len);

	memcpy(builder->string.data + builder->string.len, string.data, string.len * sizeof(char));
	builder->string.len += string.len;
}

void string_builder_deinit(string_builder_t *builder) {
	dealloc(builder->allocator, builder->string.data);
	builder->string.len = 0;
	builder->capacity = 0;
}

bool is_alphabetic(char c) {
	if ((c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z')) {
		return true;
	}
	return false;
}

bool is_digit(char c) {
	if (c >= '0' && c <= '9') {
		return true;
	}
	return false;
}

bool is_alphanumeric(char c) {
	return (is_alphabetic(c) || is_digit(c));
}

bool is_whitespace(char c) {
	return (c == ' ' || c == '\t');
}
