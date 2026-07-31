#pragma once

#include <stdio.h>
#include <stdint.h>

#include <zlib.h> // TODO: I want to abstract this away eventually
// Although it's not gonna make a difference anytime soon

#include "string.h"


typedef struct writer {
	FILE *file;

	uint64_t bytes_written;
	uint32_t crc;
	bool valid;

	bool compression_enabled;

	z_stream zlib;

	// void *context;
} writer_t;

typedef struct reader {
	FILE *file;
	bool valid;
} reader_t;

// Writer
writer_t writer_open(const string_t path);

void writer_close(writer_t *writer);

bool writer_seek(writer_t *writer, uint64_t pos);

uint64_t writer_tell(writer_t *writer);

bool writer_start_compression(writer_t *writer);

void writer_end_compression(writer_t *writer);

int write_u8(writer_t *writer, const uint8_t value);

int write_u16(writer_t *writer, const uint16_t value);

int write_u32(writer_t *writer, const uint32_t value);

int write_i32(writer_t *writer, const int32_t value);

int write_u64(writer_t *writer, const uint64_t value);

int write_bytes(writer_t *writer, const void *data, const size_t len);

// Reader
reader_t reader_open(string_t path);

void reader_close(reader_t *reader);

int read_u8(const reader_t *reader, uint8_t *value);

int read_u16(const reader_t *reader, uint16_t *value);

int read_u32(const reader_t *reader, uint32_t *value);

int read_i32(const reader_t *reader, int32_t *value);

int read_u64(const reader_t *reader, uint64_t *value);

int read_bytes(const reader_t *reader, void *data, const size_t len);

// Meta and compression
uint64_t compute_crc32(uint64_t crc, uint8_t *data, uint64_t len);

// string_t compress_buffer(allocator_t allocator, uint8_t *buffer, uint64_t len);

// string_t decompress_buffer(allocator_t allocator, uint8_t *buffer, uint64_t len);
