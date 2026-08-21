#pragma once

// TODO: implement all the other read and write functions for signed integers
// and maybe even floats

#include <stdio.h>
#include <stdint.h>

#include <zlib.h> // TODO: I want to abstract this away eventually
// Although it's not gonna make a difference anytime soon

#include "base.h"
#include "string.h"

typedef enum chunk_type {
	CHUNK_SPRITESHEET = 0,
	CHUNK_SPRITES = 1,
	CHUNK_MAP = 2,
	CHUNK_INSTRUMENTS = 3,
	CHUNK_PATTERNS = 4,
	CHUNK_ARRANGEMENTS = 5,
	CHUNK_CODE = 6,
	CHUNK_ENTITIES = 7,
} chunk_type_t;

typedef struct chunk_header {
	chunk_type_t type;

	uint64_t uncompressed_size;
	uint64_t compressed_size;
	uint32_t crc;
} chunk_header_t;

typedef struct chunk {
	uint8_t *data;
	size_t len;
	size_t capacity;
} chunk_t;

// typedef struct payload_meta {
// 	uint32_t crc;
// 	uint64_t uncompressed_size;
// 	uint64_t compressed_size;
// } payload_meta_t;

typedef struct writer {
	FILE *file;
// 	payload_meta_t *payload_meta;
} writer_t;

typedef struct reader {
	FILE *file;
// 	payload_meta_t *payload_meta;
} reader_t;

// Writer
int writer_open(writer_t *writer, const string_t path);

void writer_close(writer_t *writer);

bool writer_seek(writer_t *writer, uint64_t pos);

uint64_t writer_tell(writer_t *writer);

int write_bytes(writer_t *writer, const void *data, const size_t len);

int write_u8(writer_t *writer, const uint8_t value);

int write_u16(writer_t *writer, const uint16_t value);

int write_u32(writer_t *writer, const uint32_t value);

int write_i32(writer_t *writer, const int32_t value);

int write_u64(writer_t *writer, const uint64_t value);

int write_string(writer_t *writer, const string_t string);

int write_chunk(writer_t *writer, const chunk_type_t type, string_t data);

// Reader
reader_t reader_open(string_t path);

void reader_close(reader_t *reader);

int read_bytes(reader_t *reader, void *data, const size_t len);

int read_u8(reader_t *reader, uint8_t *value);

int read_u16(reader_t *reader, uint16_t *value);

int read_u32(reader_t *reader, uint32_t *value);

int read_i32(reader_t *reader, int32_t *value);

int read_u64(reader_t *reader, uint64_t *value);

uint32_t crc(uint32_t crc, string_t string);

string_t zip(allocator_t allocator, const string_t data);

string_t unzip(allocator_t allocator, const string_t data, const size_t unzipped_len);

// --- CHUNK ---

// int chunk_write_u8(chunk_t *chunk, const uint8_t value);
