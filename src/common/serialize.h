#pragma once

// TODO: implement all the other read and write functions for signed integers
// and maybe even floats

#include <stdio.h>
#include <stdint.h>

#include <zlib.h> // TODO: I want to abstract this away eventually
// Although it's not gonna make a difference anytime soon

#include "base.h"
#include "string.h"

#define ZLIB_BUFFER_SIZE 16384

typedef struct payload_meta {
	uint32_t crc;
	uint64_t uncompressed_size;
	uint64_t compressed_size;
} payload_meta_t;

typedef struct writer {
	bool valid;

	FILE *file;

	payload_meta_t *payload_meta;

	bool compression_enabled;
	// zlib
	z_stream zlib_stream;
	uint8_t zlib_buffer[ZLIB_BUFFER_SIZE];
} writer_t;

typedef struct reader {
	FILE *file;
	bool valid;

	payload_meta_t *payload_meta;

	bool decompression_enabled;
	// zlib
	z_stream zlib_stream;
	uint8_t zlib_buffer[ZLIB_BUFFER_SIZE];
} reader_t;

// Writer
writer_t writer_open(const string_t path);

void writer_close(writer_t *writer);

bool writer_seek(writer_t *writer, uint64_t pos);

uint64_t writer_tell(writer_t *writer);

void writer_begin_compression(writer_t *writer, payload_meta_t *payload_meta);

void writer_end_compression(writer_t *writer);

// void writer_track_payload(writer_t *writer, payload_meta_t *payload_meta);

// void reader_track_payload(reader_t *reader, payload_meta_t *payload_meta);

int write_bytes(writer_t *writer, const void *data, const size_t len);

int write_u8(writer_t *writer, const uint8_t value);

int write_u16(writer_t *writer, const uint16_t value);

int write_u32(writer_t *writer, const uint32_t value);

int write_i32(writer_t *writer, const int32_t value);

int write_u64(writer_t *writer, const uint64_t value);

// Reader
reader_t reader_open(string_t path);

void reader_close(reader_t *reader);

void reader_begin_decompression(reader_t *reader, payload_meta_t *payload_meta);

void reader_end_decompression(reader_t *reader);

int read_bytes(reader_t *reader, void *data, const size_t len);

int read_u8(reader_t *reader, uint8_t *value);

int read_u16(reader_t *reader, uint16_t *value);

int read_u32(reader_t *reader, uint32_t *value);

int read_i32(reader_t *reader, int32_t *value);

int read_u64(reader_t *reader, uint64_t *value);

// Meta and compression
// uint64_t compute_crc32(uint64_t crc, uint8_t *data, uint64_t len);

// string_t compress_buffer(allocator_t allocator, uint8_t *buffer, uint64_t len);

// string_t decompress_buffer(allocator_t allocator, uint8_t *buffer, uint64_t len);
