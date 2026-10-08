#include "serialize.h"

#include <stdio.h>
#include <string.h>
#include <assert.h>

#include "common/mem.h"
#include "common/string.h"
#include "computer.h"
#include "core/file.h"
#include "common/io.h"
#include "common/serialize.h"

// TODO: add chunk count one by one each time a chunk is written and in the end rewrite the file header
// TODO: write to temp file and when successful rename to the real file
// TODO: error handling
// TODO: string names for chunks

#define MAGIC_LEN 4
#define CURRENT_FILE_VERSION 1
#define CHUNK_COUNT 8

typedef struct header {
	uint8_t magic[MAGIC_LEN];
	uint32_t version;
	uint32_t chunk_count;
} header_t;

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

typedef enum chunk_flag {
	CHUNK_FLAG_ZLIB,
} chunk_flag_t;

typedef struct chunk_header {
	chunk_type_t type;
	uint8_t name[8];
	uint32_t flags; // TODO: use this

	uint64_t uncompressed_size;
	uint64_t compressed_size;
	uint32_t crc;
} chunk_header_t;

// --- Higher level ---

static void _write_header(writer_t *writer, const header_t *header) {
	write_bytes(writer, header->magic, MAGIC_LEN);
	write_u32(writer, header->version);
	write_u32(writer, header->chunk_count);
}

static void _write_chunk_header(writer_t *writer, const chunk_header_t header) {
	write_u8(writer, header.type);
	write_u64(writer, header.uncompressed_size);
	write_u64(writer, header.compressed_size);
	write_u32(writer, header.crc);
}

static void _write_chunk(writer_t *writer, const chunk_type_t type, string_t data) {
	// Compress
	string_t compressed = zip(get_heap_allocator(), data);

	chunk_header_t header = {
		.type = type,
		.uncompressed_size = data.len,
		.compressed_size = compressed.len,
		.crc = crc(0, data),
	};

	_write_chunk_header(writer, header);
	if (write_string(writer, compressed) != OK) {
		printf("Could not write string to disk\n");
	}

	dealloc(get_heap_allocator(), compressed.data);
}

// Change in case I ever decide to make the color_t a larger int
static void _write_color(byte_buffer_t *buffer, const color_t value) {
	byte_buffer_write_u8(buffer, (uint8_t)value);
}

static void _write_sprite(byte_buffer_t *buffer, const sprite_t *sprite) {
	byte_buffer_write_u32(buffer, sprite->flags);
	_write_color(buffer, sprite->color_key);
}

static void _write_instrument(byte_buffer_t *buffer, const instrument_t *instrument) {
	byte_buffer_write_u8(buffer, (uint8_t)instrument->waveform);

	byte_buffer_write_u8(buffer, instrument->attack);
	byte_buffer_write_u8(buffer, instrument->decay);
	byte_buffer_write_u8(buffer, instrument->sustain);
	byte_buffer_write_u8(buffer, instrument->release);
}

static void _write_pattern_step(byte_buffer_t *buffer, const pattern_step_t *step) {
	byte_buffer_write_u8(buffer, step->instrument_index);
	byte_buffer_write_u8(buffer, step->pitch);
	byte_buffer_write_u8(buffer, step->volume);
}

static void _write_pattern(byte_buffer_t *buffer, const pattern_t *pattern) {
	for (uint64_t i = 0; i < STEPS_IN_PATTERN; i++) {
		_write_pattern_step(buffer, &pattern->steps[i]);
	}
	byte_buffer_write_u8(buffer, pattern->speed);
	byte_buffer_write_u8(buffer, pattern->volume);
}

static void _write_arrangement(byte_buffer_t *buffer, const arrangement_t *arrangement) {
	byte_buffer_write_bytes(buffer, (uint8_t *)arrangement->pattern_indices, PATTERNS_IN_ARRANGEMENT * sizeof(uint16_t));
}

static void _write_entity(byte_buffer_t *buffer, const entity_t *entity) {
	if (!entity->valid) {
		return;
	}

	byte_buffer_write_i32(buffer, entity->x);
	byte_buffer_write_i32(buffer, entity->y);
	byte_buffer_write_u16(buffer, entity->sprite);
	byte_buffer_write_u8(buffer, entity->w);
	byte_buffer_write_u8(buffer, entity->h);

	// Text data
	byte_buffer_write_u64(buffer, entity->data.string.len);
	if (entity->data.string.len > 0) {
		byte_buffer_write_bytes(buffer, entity->data.string.data, entity->data.string.len);
	}
}

// TODO: use
void create_and_write_chunk(computer_t *computer, writer_t *writer, chunk_type_t type, void (*func)(computer_t *computer, byte_buffer_t *buffer)) {
	byte_buffer_t buffer = {0};
	byte_buffer_init(&buffer, get_heap_allocator(), KB(64));

	func(computer, &buffer);

	_write_chunk(writer, type, buffer.string);

	byte_buffer_deinit(&buffer);
}

void game_save(const computer_t *computer, const string_t path) {
	writer_t writer = {0};
	writer_open(&writer, path);

	// Header
	header_t header = {
		.magic = "zinc",
		.version = CURRENT_FILE_VERSION,
		.chunk_count = CHUNK_COUNT,
	};

	uint64_t header_index = writer_tell(&writer);
	_write_header(&writer, &header);
	
	// Spritesheet
	{
		byte_buffer_t buffer = {0};
		byte_buffer_init(&buffer, get_heap_allocator(), SPRITESHEET_WIDTH * SPRITESHEET_HEIGHT * sizeof(color_t));

		for (size_t y = 0; y < SPRITESHEET_HEIGHT; y++) {
			for (size_t x = 0; x < SPRITESHEET_WIDTH; x++) {
				_write_color(&buffer, computer->ram->spritesheet.data[y * SPRITESHEET_WIDTH + x]);
			}
		}

		_write_chunk(&writer, CHUNK_SPRITESHEET, buffer.string);

		byte_buffer_deinit(&buffer);
	}
	
	// Sprites
	{
		byte_buffer_t buffer = {0};
		byte_buffer_init(&buffer, get_heap_allocator(), TOTAL_SPRITES * sizeof(sprite_t));
		
		for (uint64_t i = 0; i < TOTAL_SPRITES; i++) {
			_write_sprite(&buffer, &computer->ram->sprites[i]);
		}

		_write_chunk(&writer, CHUNK_SPRITES, buffer.string);

		byte_buffer_deinit(&buffer);
	}

	// Map
	{
		byte_buffer_t buffer = {0};
		byte_buffer_init(&buffer, get_heap_allocator(), MAP_WIDTH * MAP_HEIGHT * sizeof(uint16_t));

		for (size_t i = 0; i < MAP_LAYERS_AMOUNT; i++) {
			for (size_t y = 0; y < MAP_HEIGHT; y++) {
				for (size_t x = 0; x < MAP_WIDTH; x++) {
					byte_buffer_write_u16(&buffer, computer->ram->map.layers[i].data[y * MAP_WIDTH + x]);
				}
			}
		}

		_write_chunk(&writer, CHUNK_MAP, buffer.string);

		byte_buffer_deinit(&buffer);
	}

	// Instruments
	{
		byte_buffer_t buffer = {0};
		byte_buffer_init(&buffer, get_heap_allocator(), MAX_INSTRUMENTS * sizeof(instrument_t));

		for (uint64_t i = 0; i < MAX_INSTRUMENTS; i++) {
			_write_instrument(&buffer, &computer->ram->instruments[i]);
		}

		_write_chunk(&writer, CHUNK_INSTRUMENTS, buffer.string);

		byte_buffer_deinit(&buffer);
	}

	// Patterns
	{
		byte_buffer_t buffer = {0};
		byte_buffer_init(&buffer, get_heap_allocator(), PATTERN_AMOUNT * sizeof(pattern_t));

		for (uint64_t i = 0; i < PATTERN_AMOUNT; i++) {
			_write_pattern(&buffer, &computer->ram->patterns[i]);
		}

		_write_chunk(&writer, CHUNK_PATTERNS, buffer.string);

		byte_buffer_deinit(&buffer);
	}

	// Arrangements
	{
		byte_buffer_t buffer = {0};
		byte_buffer_init(&buffer, get_heap_allocator(), MAX_ARRANGEMENTS * sizeof(arrangement_t));

		for (uint64_t i = 0; i < MAX_ARRANGEMENTS; i++) {
			_write_arrangement(&buffer, &computer->ram->arrangements[i]);
		}

		_write_chunk(&writer, CHUNK_ARRANGEMENTS, buffer.string);

		byte_buffer_deinit(&buffer);
	}

	// Code
	{
		byte_buffer_t buffer = {0};
		byte_buffer_init(&buffer, get_heap_allocator(), 8);

		byte_buffer_write_u64(&buffer, computer->active_files_amount);
		for (size_t i = 0; i < computer->active_files_amount; i++) {
			string_t code = computer->files[i].string;
			byte_buffer_write_u64(&buffer, code.len);
			byte_buffer_write_string(&buffer, code);
		}

		_write_chunk(&writer, CHUNK_CODE, buffer.string);

		byte_buffer_deinit(&buffer);
	}

	// Entities
	{
		byte_buffer_t buffer = {0};
		byte_buffer_init(&buffer, get_heap_allocator(), 8);

		// Count entities
		// TODO: make a function for this
		uint64_t entities_amount = 0;
		for (uint64_t i = 0; i < MAX_ENTITIES; i++) {
			if (computer->ram->entities.entities[i].valid) {
				entities_amount++;
			}
		}
		byte_buffer_write_u64(&buffer, entities_amount);
		for (uint64_t i = 0; i < MAX_ENTITIES; i++) {
			_write_entity(&buffer, &computer->ram->entities.entities[i]);
		}

		_write_chunk(&writer, CHUNK_ENTITIES, buffer.string);

		byte_buffer_deinit(&buffer);
	}

	writer_close(&writer);
}

static void _read_header(reader_t *reader, header_t *header) {
	if (read_bytes(reader, header->magic, MAGIC_LEN)) {
		// printf("Error reading magic number\n");
	}
	read_u32(reader, &header->version);
	read_u32(reader, &header->chunk_count);
}

void read_chunk_header(reader_t *reader, chunk_header_t *header) {
	read_u8(reader, (uint8_t *)&header->type);
	read_u64(reader, &header->uncompressed_size);
	read_u64(reader, &header->compressed_size);
	read_u32(reader, &header->crc);
}


static void _read_color(byte_buffer_t *buffer, color_t *value) {
	byte_buffer_read_u8(buffer, (uint8_t *)value);
}

static void read_sprite(byte_buffer_t *buffer, sprite_t *sprite) {
	byte_buffer_read_u32(buffer, &sprite->flags);
	_read_color(buffer, &sprite->color_key);
}

static void _read_instrument(byte_buffer_t *buffer, instrument_t *instrument) {
	byte_buffer_read_u8(buffer, (uint8_t *)&instrument->waveform);

	byte_buffer_read_u8(buffer, &instrument->attack);
	byte_buffer_read_u8(buffer, &instrument->decay);
	byte_buffer_read_u8(buffer, &instrument->sustain);
	byte_buffer_read_u8(buffer, &instrument->release);
}

static void _read_pattern_step(byte_buffer_t *buffer, pattern_step_t *step) {
	byte_buffer_read_u8(buffer, &step->instrument_index);
	byte_buffer_read_u8(buffer, &step->pitch);
	byte_buffer_read_u8(buffer, &step->volume);
}

static void _read_pattern(byte_buffer_t *buffer, pattern_t *pattern) {
	for (uint64_t i = 0; i < STEPS_IN_PATTERN; i++) {
		_read_pattern_step(buffer, &pattern->steps[i]);
	}
	byte_buffer_read_u8(buffer, &pattern->speed);
	byte_buffer_read_u8(buffer, &pattern->volume);
}

static void _read_arrangement(byte_buffer_t *buffer, arrangement_t *arrangement) {
	byte_buffer_read_bytes(buffer, (uint8_t *)arrangement->pattern_indices, PATTERNS_IN_ARRANGEMENT * sizeof(uint16_t));
}

static void _read_entity(byte_buffer_t *buffer, arena_t *arena, entity_t *entity) {
	byte_buffer_read_i32(buffer, &entity->x);
	byte_buffer_read_i32(buffer, &entity->y);
	byte_buffer_read_u16(buffer, &entity->sprite);
	byte_buffer_read_u8(buffer, &entity->w);
	byte_buffer_read_u8(buffer, &entity->h);

	// Text data
	uint64_t len = 0;
	byte_buffer_read_u64(buffer, &len);
	if (len == 0) {
		file_clear(&entity->data); // TODO: inspect this
		return;
	}

	string_t string = byte_buffer_read_string(buffer, get_arena_allocator(arena), len);

	file_clear(&entity->data);
	file_append_string(&entity->data, string);
}

// I think I want to move _write_chunk into here as well
// It's a bit too specific to this program specifically
static int _read_chunk(computer_t *computer, reader_t *reader, arena_t *arena) {
	chunk_header_t chunk_header = {0};
	read_chunk_header(reader, &chunk_header);

	printf("Type: %d\n", chunk_header.type);

	string_t compressed = read_string(reader, get_heap_allocator(), chunk_header.compressed_size);
	string_t uncompressed = unzip(get_heap_allocator(), compressed, chunk_header.uncompressed_size);
	uint32_t this_crc = crc(0, uncompressed);
	dealloc(get_heap_allocator(), compressed.data);

	// Check if crcs match
	if (this_crc != chunk_header.crc) {
		printf("Bad business is happening! The CRC calculated from the uncompressed data does not match that in the chunk header.\n");
		printf("Header crc: %u, computed crc: %u\n", chunk_header.crc, this_crc);
	
		dealloc(get_heap_allocator(), uncompressed.data);
	
		return ERR;
	}

	byte_buffer_t buffer = {0};
	byte_buffer_init_from_string(&buffer, get_heap_allocator(), uncompressed);
	dealloc(get_heap_allocator(), uncompressed.data);

	switch (chunk_header.type) {
		case CHUNK_SPRITESHEET: {
			printf("Loading spritesheet...\n");

			for (size_t y = 0; y < SPRITESHEET_HEIGHT; y++) {
				for (size_t x = 0; x < SPRITESHEET_WIDTH; x++) {
					_read_color(&buffer, &computer->ram->spritesheet.data[y * SPRITESHEET_WIDTH + x]);
					// if (computer->ram->spritesheet.data[y * SPRITESHEET_WIDTH + x] != 0) {
					// 	printf("value: %d\n", computer->ram->spritesheet.data[y * SPRITESHEET_WIDTH + x]);
					// }
				}
			}
			
			break;
		}

		case CHUNK_SPRITES: {
			printf("Loading sprite metadata...\n");

			for (size_t i = 0; i < TOTAL_SPRITES; i++) {
				read_sprite(&buffer, &computer->ram->sprites[i]);
			}

			break;
		}

		case CHUNK_MAP: {
			printf("Loading map...\n");

			for (size_t i = 0; i < MAP_LAYERS_AMOUNT; i++) {
				for (size_t y = 0; y < MAP_HEIGHT; y++) {
					for (size_t x = 0; x < MAP_WIDTH; x++) {
						byte_buffer_read_u16(&buffer, &computer->ram->map.layers[i].data[y * MAP_WIDTH + x]);
					}
				}
			}

			break;
		}

		case CHUNK_INSTRUMENTS: {
			printf("Loading instruments...\n");

			for (uint64_t i = 0; i < MAX_INSTRUMENTS; i++) {
				_read_instrument(&buffer, &computer->ram->instruments[i]);
			}

			break;
		}

		case CHUNK_PATTERNS: {
			printf("Loading patterns...\n");

			for (uint64_t i = 0; i < PATTERN_AMOUNT; i++) {
				_read_pattern(&buffer, &computer->ram->patterns[i]);
			}

			break;
		}

		case CHUNK_ARRANGEMENTS: {
			printf("Loading arrangements...\n");

			for (uint64_t i = 0; i < MAX_ARRANGEMENTS; i++) {
				_read_arrangement(&buffer, &computer->ram->arrangements[i]);
			}

			break;
		}

		case CHUNK_CODE: {
			printf("Loading code...\n");

			byte_buffer_read_u64(&buffer, &computer->active_files_amount);
			
			for (size_t i = 0; i < computer->active_files_amount; i++) {
				uint64_t len = 0;
				byte_buffer_read_u64(&buffer, &len);

				string_t code = byte_buffer_read_string(&buffer, get_arena_allocator(arena), len);
				file_append_string(&computer->files[i], code);
			}

			break;
		}

		case CHUNK_ENTITIES: {
			printf("Loading entities...\n");

			uint64_t entities_amount = 0;
			byte_buffer_read_u64(&buffer, &entities_amount);
			printf("entities_amount: %zu\n", entities_amount);
			
			for (uint64_t i = 0; i < entities_amount; i++) {
				_read_entity(&buffer, arena, &computer->ram->entities.entities[i]);
				computer->ram->entities.entities[i].valid = true;
			}

			break;
		}
	}

	return OK;
}

int game_load(computer_t *computer, string_t path) {
	if (path_is_file(path)) {
		set_game_path(computer, path);
	} else {
		return 1;
	}

	reader_t reader = reader_open(path);

	arena_t arena = {0};
	arena_init(&arena, MB(8));

	header_t header = {0};
	_read_header(&reader, &header);

	if (!(header.magic[0] == 'z' && header.magic[1] == 'i' && header.magic[2] == 'n' && header.magic[3] == 'c')) {
		// File invalid
		printf("Magic number does not match\n");
		
		return 1;
	}
	
	if (header.version != CURRENT_FILE_VERSION) {
		// Convert it or something
		printf("File version does not match\n");
	}
	
	for (size_t i = 0; i < header.chunk_count; i++) {
		if (_read_chunk(computer, &reader, &arena) == ERR) {
			break;
		}
	}

	reader_close(&reader);

	arena_free(&arena);

	return OK;
}
