#include "serialize.h"

#include <stdio.h>
#include <string.h>

#include "common/mem.h"
#include "common/string.h"
#include "computer.h"
#include "core/file.h"
#include "common/io.h"
#include "common/serialize.h"

// 3 parts
// Static
// Code
// Entities
// Each have their own crc hash

// TODO: compression?
// hash checksum
// that is stored when saving and then read when loading and also calculated
// so they can be compared

#define MAGIC_LEN 4

#define CURRENT_FILE_VERSION 1

typedef struct header {
	uint8_t magic[MAGIC_LEN];
	uint32_t version;
	uint32_t crc32_hash; // TODO: use
	
	// uint64_t data_size;
	
	// uint64_t uncompressed_size;
	// uint64_t compressed_size;
} header_t;

// --- Higher level ---

static void _write_header(writer_t *writer, const header_t *header) {
	write_bytes(writer, header->magic, MAGIC_LEN);
	write_u32(writer, header->version);
	write_u32(writer, header->crc32_hash);
}

// Change in case I ever decide to make the color_t a larger int
static int _write_color(writer_t *writer, const color_t value) {
	return write_u8(writer, (uint8_t)value);
}

static void _write_sprite(writer_t *writer, const sprite_t *sprite) {
	write_u32(writer, sprite->flags);
	_write_color(writer, sprite->color_key);
}

static void _write_entity(writer_t *writer, const entity_t *entity) {
	if (!entity->valid) {
		return;
	}

	write_i32(writer, entity->x);
	write_i32(writer, entity->y);
	write_u16(writer, entity->sprite);
	write_u8(writer, entity->w);
	write_u8(writer, entity->h);

	// Text data
	write_u64(writer, entity->data.string.len);
	if (entity->data.string.len > 0) {
		write_bytes(writer, entity->data.string.data, entity->data.string.len);
	}
}

static void _write_instrument(writer_t *writer, const instrument_t *instrument) {
	write_u8(writer, (uint8_t)instrument->waveform);

	write_u8(writer, instrument->attack);
	write_u8(writer, instrument->decay);
	write_u8(writer, instrument->sustain);
	write_u8(writer, instrument->release);
}

static void _write_pattern_step(writer_t *writer, const pattern_step_t *step) {
	write_u8(writer, step->instrument_index);
	write_u8(writer, step->pitch);
	write_u8(writer, step->volume);
}

static void _write_pattern(writer_t *writer, const pattern_t *pattern) {
	for (uint64_t i = 0; i < STEPS_IN_PATTERN; i++) {
		_write_pattern_step(writer, &pattern->steps[i]);
	}
	write_u8(writer, pattern->speed);
	write_u8(writer, pattern->volume);
}

static void _write_arrangement(writer_t *writer, const arrangement_t *arrangement) {
	write_bytes(writer, arrangement->pattern_indices, PATTERNS_IN_ARRANGEMENT * sizeof(uint16_t));
}

void game_save(const computer_t *computer, const string_t path) {
	writer_t writer = writer_open(path);

	// Header
	header_t header = {
		.magic = "zinc",
		.version = CURRENT_FILE_VERSION,
		// .crc32_hash = compute_crc32(0, uint8_t *data, uint64_t len),
	};
	_write_header(&writer, &header);

	// Code
	write_u64(&writer, computer->active_files_amount);
	for (size_t i = 0; i < computer->active_files_amount; i++) {
		string_t code = computer->files[i].string;
		write_u64(&writer, code.len);
		write_bytes(&writer, code.data, code.len);
	}

	// Spritesheet
	write_bytes(&writer, computer->ram->spritesheet.data, SPRITESHEET_WIDTH * SPRITESHEET_HEIGHT * sizeof(color_t));
	
	// Sprites
	for (uint64_t i = 0; i < TOTAL_SPRITES; i++) {
		_write_sprite(&writer, &computer->ram->sprites[i]);
	}

	// Map
	for (uint64_t i = 0; i < MAP_LAYERS_AMOUNT; i++) {
		write_bytes(&writer, computer->ram->map.layers[i].data, MAP_WIDTH * MAP_HEIGHT * sizeof(uint16_t));
	}

	// Entities
	uint64_t entities_amount = 0;
	for (uint64_t i = 0; i < MAX_ENTITIES; i++) {
		if (computer->ram->entities.entities[i].valid) {
			entities_amount++;
		}
	}
	write_u64(&writer, entities_amount);
	for (uint64_t i = 0; i < MAX_ENTITIES; i++) {
		_write_entity(&writer, &computer->ram->entities.entities[i]);
	}

	// Instruments
	for (uint64_t i = 0; i < MAX_INSTRUMENTS; i++) {
		_write_instrument(&writer, &computer->ram->instruments[i]);
	}

	// Patterns
	for (uint64_t i = 0; i < PATTERN_AMOUNT; i++) {
		_write_pattern(&writer, &computer->ram->patterns[i]);
	}

	// Arrangements
	for (uint64_t i = 0; i < MAX_ARRANGEMENTS; i++) {
		_write_arrangement(&writer, &computer->ram->arrangements[i]);
	}

	// Patch header
	

	writer_close(&writer);
}

void read_header(reader_t *reader, header_t *header) {
	read_bytes(reader, header->magic, 4);
	read_u32(reader, &header->version);
	read_u32(reader, &header->crc32_hash);
}

int read_color(reader_t *reader, color_t *value) {
	return read_u8(reader, (uint8_t *)value);
}

void read_sprite(reader_t *reader, sprite_t *sprite) {
	read_u32(reader, &sprite->flags);
	read_color(reader, &sprite->color_key);
}

void read_entity(const reader_t *reader, arena_t *arena, entity_t *entity) {
	read_i32(reader, &entity->x);
	read_i32(reader, &entity->y);
	read_u16(reader, &entity->sprite);
	read_u8(reader, &entity->w);
	read_u8(reader, &entity->h);

	// Text data
	uint64_t len = 0;
	read_u64(reader, &len);
	if (len == 0) {
		file_clear(&entity->data);
		return;
	}

	uint8_t *buffer = arena_alloc(arena, len);
	read_bytes(reader, buffer, len);
	
	string_t string = {
		.data = buffer,
		.len = len,
	};

	file_clear(&entity->data);
	file_append_string(&entity->data, string);
}

void read_instrument(const reader_t *reader, instrument_t *instrument) {
	read_u8(reader, (uint8_t *)&instrument->waveform);

	read_u8(reader, &instrument->attack);
	read_u8(reader, &instrument->decay);
	read_u8(reader, &instrument->sustain);
	read_u8(reader, &instrument->release);
}

void read_pattern_step(const reader_t *reader, pattern_step_t *step) {
	read_u8(reader, &step->instrument_index);
	read_u8(reader, &step->pitch);
	read_u8(reader, &step->volume);
}

void read_pattern(const reader_t *reader, pattern_t *pattern) {
	for (uint64_t i = 0; i < STEPS_IN_PATTERN; i++) {
		read_pattern_step(reader, &pattern->steps[i]);
	}
	read_u8(reader, &pattern->speed);
	read_u8(reader, &pattern->volume);
}

void read_arrangement(const reader_t *reader, arrangement_t *arrangement) {
	read_bytes(reader, arrangement->pattern_indices, PATTERNS_IN_ARRANGEMENT * sizeof(uint16_t));
}

int game_load(computer_t *computer, string_t path) {
	// game_load_text(computer, path);
	// return 0;

	if (path_is_file(path)) {
		set_game_path(computer, path);
	} else {
		return 1;
	}

	reader_t reader = reader_open(path);

	arena_t arena = {0};
	arena_init(&arena, MB(8));

	header_t header = {0};
	read_header(&reader, &header);

	if (!(header.magic[0] == 'z' && header.magic[1] == 'i' && header.magic[2] == 'n' && header.magic[3] == 'c')) {
		// File invalid
		
		return 1;
	}

	if (header.version != CURRENT_FILE_VERSION) {
		// Convert it or something
	}

	// Code
	read_u64(&reader, &computer->active_files_amount);
	for (uint64_t i = 0; i < computer->active_files_amount; i++) {
		uint64_t len = 0;
		read_u64(&reader, &len);
		
		uint8_t *buffer = arena_alloc(&arena, len);
		read_bytes(&reader, buffer, len);
		
		string_t string = {
			.data = buffer,
			.len = len,
		};

		file_append_string(&computer->files[i], string);
	}

	// Spritesheet
	read_bytes(&reader, computer->ram->spritesheet.data, SPRITESHEET_WIDTH * SPRITESHEET_HEIGHT * sizeof(color_t));

	// Sprites
	for (uint64_t i = 0; i < TOTAL_SPRITES; i++) {
		read_sprite(&reader, &computer->ram->sprites[i]);
	}

	// Map
	for (uint64_t i = 0; i < MAP_LAYERS_AMOUNT; i++) {
		read_bytes(&reader, computer->ram->map.layers[i].data, MAP_WIDTH * MAP_HEIGHT * sizeof(uint16_t));
	}

	// Entities
	uint64_t entities_amount = 0;
	read_u64(&reader, &entities_amount);
	for (uint64_t i = 0; i < entities_amount; i++) {
		read_entity(&reader, &arena, &computer->ram->entities.entities[i]);
		computer->ram->entities.entities[i].valid = true;
	}

	// Instruments
	for (uint64_t i = 0; i < MAX_INSTRUMENTS; i++) {
		read_instrument(&reader, &computer->ram->instruments[i]);
	}

	// Patterns
	for (uint64_t i = 0; i < PATTERN_AMOUNT; i++) {
		read_pattern(&reader, &computer->ram->patterns[i]);
	}

	// Arrangements
	for (uint64_t i = 0; i < MAX_ARRANGEMENTS; i++) {
		read_arrangement(&reader, &computer->ram->arrangements[i]);
	}

	reader_close(&reader);

	arena_free(&arena);

	return 0;
}
