#include "serialize.h"

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
	return (uint64_t)ftell(writer->file);
}

// void writer_track_payload(writer_t *writer, payload_meta_t *payload_meta) {
// 	payload_meta->crc = 0;
// 	payload_meta->uncompressed_size = 0;
// 	payload_meta->compressed_size = 0;

// 	writer->payload_meta = payload_meta;
// }

// void reader_track_payload(reader_t *reader, payload_meta_t *payload_meta) {
// 	payload_meta->crc = 0;
// 	payload_meta->uncompressed_size = 0;
// 	payload_meta->compressed_size = 0;

// 	reader->payload_meta = payload_meta;
// }

// Returns 0 if correct (like most C programs I think)
int write_bytes(writer_t *writer, const void *data, const size_t len) {
	if (!writer->compression_enabled) {
		// TODO: check if this return value makes sense
		return fwrite(data, 1, len, writer->file) != len;
	}

	assert(writer->payload_meta != NULL);

	writer->payload_meta->uncompressed_size += len;
	
	writer->payload_meta->crc = crc32(writer->payload_meta->crc, data, len);

	writer->zlib_stream.next_in = (Bytef *)data;
	writer->zlib_stream.avail_in = (uInt)len;

	while (writer->zlib_stream.avail_in > 0) {
		writer->zlib_stream.next_out = writer->zlib_buffer;
		writer->zlib_stream.avail_out = ZLIB_BUFFER_SIZE;

		int ret = deflate(&writer->zlib_stream, Z_NO_FLUSH);
		if (ret != Z_OK) {
			return ERR;
		}

		size_t produced = ZLIB_BUFFER_SIZE - writer->zlib_stream.avail_out;

		writer->payload_meta->compressed_size += produced;

		if (produced && (fwrite(writer->zlib_buffer, 1, produced, writer->file) != produced)) {
			return ERR;
		}
	}

	return OK;
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

int read_bytes(reader_t *reader, void *data, const size_t len) {
	if (!reader->decompression_enabled) {
		return fread(data, 1, len, reader->file) != len;
	}
	assert(reader->payload_meta != NULL);

	reader->zlib_stream.next_out = (Bytef *)data;
	reader->zlib_stream.avail_out = (uInt)len;

	Bytef *out = (Bytef *)data;
	uint64_t remaining = len;

	while (remaining > 0) {
		reader->zlib_stream.next_out = out;
		reader->zlib_stream.avail_out = (uInt)remaining;

		if (reader->zlib_stream.avail_in == 0) {
			size_t read = fread(reader->zlib_buffer, 1, ZLIB_BUFFER_SIZE, reader->file);

			if (read == 0) {
				return ERR;
			}

			reader->payload_meta->compressed_size += read;

			reader->zlib_stream.next_in = (Bytef *)reader->zlib_buffer;
			reader->zlib_stream.avail_in = (uInt)read;
		}

		int before = reader->zlib_stream.avail_out;

		int ret = inflate(&reader->zlib_stream, Z_NO_FLUSH);

		if (ret == Z_STREAM_END) {
			// compressed block ended before filling requested data
			return reader->zlib_stream.avail_out == 0;
		}

		if (ret != Z_OK) {
			return ERR;
		}

		int produced = before - reader->zlib_stream.avail_out;
		if (produced > 0) {
			reader->payload_meta->uncompressed_size += produced;

			reader->payload_meta->crc = crc32(reader->payload_meta->crc, out,produced);
			out += produced;
			remaining -= produced;
		}
	}

	// return OK;
	return remaining == 0 ? OK : ERR;
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

void writer_begin_compression(writer_t *writer, payload_meta_t *payload_meta) {
	if (writer->compression_enabled) {
		return;
	}

	payload_meta->crc = 0;
	payload_meta->uncompressed_size = 0;
	payload_meta->compressed_size = 0;
	writer->payload_meta = payload_meta;

	writer->payload_meta->crc = crc32(0L, Z_NULL, 0);

	writer->zlib_stream.zalloc = Z_NULL;
	writer->zlib_stream.zfree = Z_NULL;
	writer->zlib_stream.opaque = Z_NULL;

	writer->zlib_stream.avail_in = 0;
	writer->zlib_stream.next_in = Z_NULL;
	
	if (deflateInit(&writer->zlib_stream, Z_DEFAULT_COMPRESSION) != Z_OK) {
		writer->valid = false;
		return;
	}

	writer->compression_enabled = true;
}

// TODO FOR TOMORROW: compute crc and bytes read and stuff for reader
// I should probably also store what compression algorithm is being used

// Maybe this function should return crc, uncompressed size, compressed size
// or just keep it part of the writer struct I think that might be easier
void writer_end_compression(writer_t *writer) {
	assert(writer->payload_meta != NULL);

	if (!writer->compression_enabled) {
		return;
	}

	int ret = 0;

	do {
		writer->zlib_stream.next_out = writer->zlib_buffer;
		writer->zlib_stream.avail_out = ZLIB_BUFFER_SIZE;
		
		ret = deflate(&writer->zlib_stream, Z_FINISH);
		if (ret != Z_OK && ret != Z_STREAM_END) {
			writer->valid = false;
			break;
		}

		size_t produced = ZLIB_BUFFER_SIZE - writer->zlib_stream.avail_out;

		if (produced) {
			if (fwrite(writer->zlib_buffer, 1, produced, writer->file) != produced) {
				writer->valid = false;
				break;
			}

			writer->payload_meta->compressed_size += produced;
		}
	} while (ret == Z_OK);

	deflateEnd(&writer->zlib_stream);

	writer->compression_enabled = false;
}

void reader_begin_decompression(reader_t *reader, payload_meta_t *payload_meta) {
	if (reader->decompression_enabled) {
		return;
	}

	payload_meta->crc = 0;
	payload_meta->uncompressed_size = 0;
	payload_meta->compressed_size = 0;
	reader->payload_meta = payload_meta;

	reader->zlib_stream.zalloc = Z_NULL;
	reader->zlib_stream.zfree = Z_NULL;
	reader->zlib_stream.opaque = Z_NULL;

	reader->zlib_stream.avail_in = 0;
	reader->zlib_stream.next_in = Z_NULL;

	 if (inflateInit(&reader->zlib_stream) != Z_OK) {
		reader->valid = false;
		return;
	}

	reader->decompression_enabled = true;
}

void reader_end_decompression(reader_t *reader) {
	if (!reader->decompression_enabled) {
		return;
	}

	inflateEnd(&reader->zlib_stream);

	reader->decompression_enabled = false;
}
