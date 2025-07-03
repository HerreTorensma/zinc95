#include "string.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "mem.h"

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