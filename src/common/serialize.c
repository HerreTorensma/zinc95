#include "serialize.h"
#include "mem.h"
#include "string.h"

#include <stdint.h>
#include <stdio.h>
#include <assert.h>

#include <zlib.h>

int writer_open(writer_t *writer, const string_t path) {
	char *c_path = string_to_c_string(get_temp_allocator(), path);
	writer->file = fopen(c_path, "wb");

	if (!writer->file) {
		printf("Could not open %s for writing\n", c_path);
		return ERR;
	}

	return OK;
}

void writer_close(writer_t *writer) {
	fclose(writer->file);
}

bool writer_seek(writer_t *writer, uint64_t pos) {
	return fseek(writer->file, pos, SEEK_SET) == 0;
}

uint64_t writer_tell(writer_t *writer) {
	return (uint64_t)ftell(writer->file);
}

// Returns 0 if correct (like most C programs I think)
int write_bytes(writer_t *writer, const void *data, const size_t len) {
	if (len == 0) {
		return OK;
	}
	if (!writer || !writer->file || !data) {
		return ERR;
	}

	if (fwrite(data, 1, len, writer->file) != len) {
		return ERR;
	}
	return OK;
}

int write_string(writer_t *writer, const string_t string) {
	return write_bytes(writer, string.data, string.len);
}

// Writer helper functions
int write_u8(writer_t *writer, const uint8_t value) {
	return write_bytes(writer, &value, sizeof(uint8_t));
}

int write_u16(writer_t *writer, const uint16_t value) {
	return write_bytes(writer, &value, sizeof(uint16_t));
}

int write_u32(writer_t *writer, const uint32_t value) {
	return write_bytes(writer, &value, sizeof(uint32_t));
}

int write_i32(writer_t *writer, const int32_t value) {
	return write_bytes(writer, &value, sizeof(int32_t));
}

int write_u64(writer_t *writer, const uint64_t value) {
	return write_bytes(writer, &value, sizeof(uint64_t));
}

reader_t reader_open(string_t path) {
	reader_t reader = {0};

	char *c_path = string_to_c_string(get_temp_allocator(), path);
	reader.file = fopen(c_path, "rb");

	if (!reader.file) {
		printf("Could not open %s for reading\n", c_path);
	}

	return reader;
}

void reader_close(reader_t *reader) {
	fclose(reader->file);
}

int read_bytes(reader_t *reader, void *data, const size_t len) {
	return fread(data, 1, len, reader->file);
}

string_t read_string(reader_t *reader, allocator_t allocator, const size_t len) {
	string_t string = {0};
	string.data = alloc(allocator, len);
	string.len = len;
	read_bytes(reader, string.data, len);
	return string;
}

int read_u8(reader_t *reader, uint8_t *value) {
	return read_bytes(reader, value, sizeof(uint8_t));
}

int read_u16(reader_t *reader, uint16_t *value) {
	return read_bytes(reader, value, sizeof(uint16_t));
}

int read_u32(reader_t *reader, uint32_t *value) {
	return read_bytes(reader, value, sizeof(uint32_t));
}

int read_i32(reader_t *reader, int32_t *value) {
	return read_bytes(reader, value, sizeof(int32_t));
}

int read_u64(reader_t *reader, uint64_t *value) {
	return read_bytes(reader, value, sizeof(uint64_t));
}
