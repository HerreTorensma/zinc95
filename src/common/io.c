#include "io.h"

#include <stdio.h>
#include <assert.h>

void file_write_string(string_t path, string_t string) {
	char *c_path = string_to_c_string(get_temp_allocator(), path);
	FILE *file = fopen(c_path, "wb");

	if (!file) {
		printf("Could not open %s for writing\n", c_path);
		return;
	}

	fwrite(string.data, sizeof(uint8_t), string.len, file);
	fclose(file);
}

string_t file_load_to_string(allocator_t allocator, string_t path) {
	FILE *file = fopen(string_to_c_string(get_temp_allocator(), path), "r");
	if (file == NULL) {
		printf("Unable to open file\n");
		return (string_t){.data = NULL, .len = 0};
	}

	fseek(file, 0, SEEK_END);
	size_t file_size = ftell(file);
	fseek(file, 0, SEEK_SET);

	string_t string = {
		.data = alloc(allocator, file_size),
		.len = file_size,
	};
	assert(string.data != NULL);

	size_t read_len = fread(string.data, sizeof(uint8_t), file_size, file);
	// assert(read_len == file_size);
	
	fclose(file);

	return string;
}
