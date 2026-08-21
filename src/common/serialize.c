#include "serialize.h"
#include "mem.h"
#include "string.h"

#include <stdint.h>
#include <stdio.h>
#include <assert.h>

#include <zlib.h>

// I think I'll get rid of the streaming compression and writing
// And instead just have the writer store stuff until the actual writer_write() or something is called
// Then the reader also reads everything into a buffer, and decompresses the whole thing at once
// I think that will be way simpler than this streaming stuff but I'll have to see if the performance is good enough
// Also I want to still do a chunk system

// Its called a memory writer

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

int write_chunk_header(writer_t *writer, const chunk_header_t header) {
	// chunk_type_t type;

	// uint32_t crc;
	// uint64_t uncompressed_size;
	// uint64_t compressed_size;
	write_u8(writer, header.type);
	write_u64(writer, header.uncompressed_size);
	write_u64(writer, header.compressed_size);
	write_u32(writer, header.crc);
}

int write_chunk(writer_t *writer, const chunk_type_t type, string_t data) {
	// Compress
	string_t compressed = zip(get_heap_allocator(), data);

	chunk_header_t header = {
		.compressed_size = compressed.len,
		.uncompressed_size = data.len,
		.crc = crc(0, data),
	};

	write_chunk_header(writer, header);
	write_string(writer, compressed);
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

}

string_t read_string(allocator_t allocator, const size_t len) {

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

void read_chunk_header(reader_t *reader, chunk_header_t *header) {

}

int buffer_read_bytes(string_builder_t *builder, void *data, const size_t len) {

}

int buffer_read_u8(string_builder_t *builder, uint8_t *value) {
	
}

// --- CHUNK ---

// static void _chunk_reserve(chunk_t *chunk, size_t needed_capacity) {
// 	if (chunk->capacity >= needed_capacity) {
// 		return;
// 	}

// 	size_t old_capacity = chunk->capacity;

// 	chunk->capacity = get_next_power_of_2(needed_capacity);

// 	char *new_data = heap_alloc(chunk->capacity * sizeof(char));
// 	memcpy(new_data, chunk->string.data, old_capacity * sizeof(char));

// 	if (chunk->string.data != NULL) {
// 		heap_dealloc(chunk->string.data);
// 	}

// 	chunk->string.data = new_data;
// }

// int chunk_write_u8(chunk_t *chunk, const uint8_t value) {

// }