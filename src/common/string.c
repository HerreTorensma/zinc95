#include "string.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

// void string_array_push(string_array_t *array, string_t string) {
// 	array->data[array->size] = string;
// 	array->size++;
// }

string_t temp_alloc_string(size_t capacity) {
	return (string_t) {
		.data = temp_alloc(capacity * sizeof(char)),
		// TODO: why is the len 0? there was a reason for it but i don't remember
		// its me from the future of course the len is zero since you're allocating an empty string
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

// Assumes the dest string has enough memory allocated
// TODO: string builder stuff
void string_append(string_t *dest, string_t src) {
	size_t old_len = dest->len;
	dest->len += src.len;

	memcpy(dest->data + old_len, src.data, src.len);
}

string_t string_view(string_t source, size_t start, size_t len) {
	return (string_t){
		.data = source.data + start,
		.len = len,
	};
}

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
