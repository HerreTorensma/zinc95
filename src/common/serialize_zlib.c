#include "mem.h"
#include "serialize.h"

#include <zlib.h>

uint64_t compute_crc32(uint64_t crc, uint8_t *data, uint64_t len) {
	return crc32(crc, data, len);
}

// string_t compress_buffer(allocator_t allocator, uint8_t *buffer, uint64_t len) {
// 	uLong compressed_len = compressBound(len);
// 	uint8_t *compressed = alloc(allocator, compressed_len);

// 	int ret = compress(compressed, &compressed_len, buffer, len);
// 	if (ret != Z_OK) {
// 		// Handle
// 	}

// 	return (string_t){
// 		.data = compressed,
// 		.len = compressed_len,
// 	};
// }

// string_t decompress_buffer(allocator_t allocator, uint8_t *buffer, uint64_t len) {

// }

bool writer_start_compression(writer_t *writer) {
	
}

void writer_end_compression(writer_t *writer) {

}