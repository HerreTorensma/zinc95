#pragma once

// TODO: implement all the other read and write functions for signed integers
// and maybe even floats

#include <stdio.h>
#include <stdint.h>

#include "base.h"
#include "string.h"

typedef struct writer {
	FILE *file;
} writer_t;

typedef struct reader {
	FILE *file;
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

// Reader
reader_t reader_open(string_t path);

void reader_close(reader_t *reader);

int read_bytes(reader_t *reader, void *data, const size_t len);

string_t read_string(reader_t *reader, allocator_t allocator, const size_t len);

int read_u8(reader_t *reader, uint8_t *value);

int read_u16(reader_t *reader, uint16_t *value);

int read_u32(reader_t *reader, uint32_t *value);

int read_i32(reader_t *reader, int32_t *value);

int read_u64(reader_t *reader, uint64_t *value);

uint32_t crc(uint32_t crc, string_t string);

string_t zip(allocator_t allocator, const string_t data);

string_t unzip(allocator_t allocator, const string_t data, const size_t unzipped_len);
