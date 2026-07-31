#include "serialize.h"
#include <stdio.h>

writer_t writer_open(const string_t path) {
	writer_t writer = {0};
	writer.valid = true;

	char *c_path = string_to_c_string(get_temp_allocator(), path);
	writer.file = fopen(c_path, "wb");

	if (!writer.file) {
		printf("Could not open %s for writing\n", c_path);
		writer.valid = false;
	}

	return writer;
}

void writer_close(writer_t *writer) {
	fclose(writer->file);
}

bool writer_seek(writer_t *writer, uint64_t pos) {
	return fseek(writer->file, pos, SEEK_SET) == 0;
}

uint64_t writer_tell(writer_t *writer) {
	return ftell(writer->file);
}

// Writer helper functions
int write_u8(writer_t *writer, const uint8_t value) {
	int ret = fwrite(&value, sizeof(value), 1, writer->file);
	
	if (ret == 1) {
		writer->bytes_written += sizeof(value);

		return true;
	}

	return false;
}

int write_u16(writer_t *writer, const uint16_t value) {
	return fwrite(&value, sizeof(uint16_t), 1, writer->file) == 1;
}

int write_u32(writer_t *writer, const uint32_t value) {
	return fwrite(&value, sizeof(uint32_t), 1, writer->file) == 1;
}

int write_i32(writer_t *writer, const int32_t value) {
	return fwrite(&value, sizeof(int32_t), 1, writer->file) == 1;
}

int write_u64(writer_t *writer, const uint64_t value) {
	return fwrite(&value, sizeof(uint64_t), 1, writer->file) == 1;
}

int write_bytes(writer_t *writer, const void *data, const size_t len) {
	return fwrite(data, 1, len, writer->file) == len;
}

reader_t reader_open(string_t path) {
	reader_t reader = {0};
	reader.valid = true;

	char *c_path = string_to_c_string(get_temp_allocator(), path);
	reader.file = fopen(c_path, "rb");

	if (!reader.file) {
		printf("Could not open %s for reading\n", c_path);
		reader.valid = false;
	}

	return reader;
}

void reader_close(reader_t *reader) {
	fclose(reader->file);
}

int read_u8(const reader_t *reader, uint8_t *value) {
	return fread(value, sizeof(uint8_t), 1, reader->file) == 1;
}

int read_u16(const reader_t *reader, uint16_t *value) {
	return fread(value, sizeof(uint16_t), 1, reader->file) == 1;
}

int read_u32(const reader_t *reader, uint32_t *value) {
	return fread(value, sizeof(uint32_t), 1, reader->file) == 1;
}

int read_i32(const reader_t *reader, int32_t *value) {
	return fread(value, sizeof(int32_t), 1, reader->file) == 1;
}

int read_u64(const reader_t *reader, uint64_t *value) {
	return fread(value, sizeof(uint64_t), 1, reader->file) == 1;
}

int read_bytes(const reader_t *reader, void *data, const size_t len) {
	return fread(data, 1, len, reader->file) == len;
}
