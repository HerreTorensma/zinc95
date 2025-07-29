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

void string_place(string_t *base, string_t new_string) {
	memcpy(base->data, new_string.data, new_string.len);
	base->len = new_string.len;
}

bool string_is_empty(string_t string) {
	if (string.len == 0) {
		return true;
	}

	return false;
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

string_t int_to_string(allocator_t allocator, int n) {
	string_t string = {
		// -2147483648: 11 bytes
		.data = alloc(allocator, 11),
		.len = 0,
	};

	if (n == 0) {
		string.data[0] = '0';
		string.len = 1;
		return string;
	}
	
	bool is_signed = false;
	if (n < 0) {
		n = -n;
		is_signed = true;
	}

	while (n > 0) {
		string.data[string.len] = n % 10 + '0';
		string.len++;
		n /= 10;
	}

	if (is_signed) {
		string.data[string.len] = '-';
		string.len++;
	}

	// Reverse the string
	for (size_t i = 0; i < string.len / 2; i++) {
		char temp = string.data[i];
		string.data[i] = string.data[string.len - i - 1];
		string.data[string.len - i - 1] = temp;
	}

	return string;
}

int string_to_int(string_t string) {
	int n = 0;

	for (size_t i = 0; i < string.len; i++) {
		if (is_digit(string.data[i])) {
			n *= 10;
			n += string.data[i] - '0';
		}
	}

	return n;
}

string_t path_append(allocator_t allocator, string_t base, string_t appendage) {
	if (base.len == 0 && appendage.len == 0) {
		return (string_t){0};
	}
	if (base.len == 0) {
		return string_copy(allocator, appendage);
	}
	if (appendage.len == 0) {
		return string_copy(allocator, base);
	}

	bool base_ends_in_slash = base.data[base.len - 1] == '/';
	bool appendage_starts_with_slash = appendage.data[0] == '/';

	if (base_ends_in_slash && appendage_starts_with_slash) {
		// Copy appendage from index 1
		string_t new_string = {
			.data = alloc(allocator, base.len + (appendage.len - 1)),
			.len = base.len + (appendage.len - 1),
		};

		memcpy(new_string.data, base.data, base.len);
		memcpy(new_string.data + base.len, appendage.data + 1, appendage.len - 1);

		return new_string;
	}

	if ((base_ends_in_slash && !appendage_starts_with_slash) || (!base_ends_in_slash && appendage_starts_with_slash)) {
		// Normal concatenation
		string_t new_string = {
			.data = alloc(allocator, base.len + appendage.len),
			.len = base.len + appendage.len,
		};
		
		memcpy(new_string.data, base.data, base.len);
		memcpy(new_string.data + base.len, appendage.data, appendage.len);

		return new_string;
	}

	// Else: copy base, add a /, copy appendage
	// if (!base_ends_in_slash && !appendage_starts_with_slash) {
	string_t new_string = {
		.data = alloc(allocator, base.len + appendage.len + 1),
		.len = base.len + appendage.len + 1,
	};
	memcpy(new_string.data, base.data, base.len);
	new_string.data[base.len] = '/';
	memcpy(new_string.data + base.len + 1, appendage.data, appendage.len);
	return new_string;
	// }
}

string_t path_get_truncated_view(string_t path) {
	if (path.len == 0) {
		return path;
	}

	if (path.len == 1 && path.data[0] == '/') {
		return path;
	}

	// TODO: define ssize_t myself and use
	for (int64_t i = path.len - 1; i >= 0; i--) {
		if (path.data[i] == '/') {
			if (i == 0) {
				path.len = 1;
			} else {
				path.len = i;
			}
			break;
		}
	}

	return path;
}
