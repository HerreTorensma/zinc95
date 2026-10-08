#include "serialize.h"

#include <zlib.h>

uint32_t crc(uint32_t crc, string_t string) {
	return crc32(crc, string.data, string.len);
}

string_t zip(allocator_t allocator, const string_t data) {
	uLongf compressed_len = compressBound(data.len);
	uint8_t *buffer = alloc(allocator, compressed_len);

	// TODO: error handling
	int ret = compress2(buffer, &compressed_len, data.data, (uLong)data.len, Z_BEST_COMPRESSION);
	if (ret != Z_OK) {
		dealloc(allocator, buffer);
		return (string_t){0};
	}

	return (string_t){.data = buffer, .len = compressed_len};
}

string_t unzip(allocator_t allocator, const string_t data, const size_t unzipped_len) {
	uint8_t *buffer = alloc(allocator, unzipped_len);

	// TODO: error handling
	uLongf dest_len = unzipped_len;
	int ret = uncompress(buffer, &dest_len, data.data, (uLong)data.len);
	if (ret != Z_OK) {
		dealloc(allocator, buffer);
		return (string_t){0};
	}

	return (string_t){.data = buffer, .len = dest_len};
}
