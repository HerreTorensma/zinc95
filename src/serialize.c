#include "serialize.h"

#include <stdio.h>
#include <string.h>

#include "common/mem.h"
#include "common/string.h"
#include "computer.h"
#include "core/file.h"
#include "common/io.h"

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
} header_t;

uint64_t crc32(uint8_t *data, uint64_t len) {
	return 0;
}

typedef struct writer {
	FILE *file;
	bool valid;
} writer_t;

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

// Writer helper functions
int write_u8(const writer_t *writer, const uint8_t value) {
	return fwrite(&value, sizeof(uint8_t), 1, writer->file) == 1;
}

int write_u16(const writer_t *writer, const uint8_t value) {
	return fwrite(&value, sizeof(uint16_t), 1, writer->file) == 1;
}

int write_u32(const writer_t *writer, const uint32_t value) {
	return fwrite(&value, sizeof(uint32_t), 1, writer->file) == 1;
}

int write_i32(const writer_t *writer, const int32_t value) {
	return fwrite(&value, sizeof(int32_t), 1, writer->file) == 1;
}

int write_u64(const writer_t *writer, const uint64_t value) {
	return fwrite(&value, sizeof(uint64_t), 1, writer->file) == 1;
}

int write_bytes(const writer_t *writer, const void *data, const size_t len) {
	return fwrite(data, 1, len, writer->file) == len;
}

// --- READER ---

typedef struct reader {
	FILE *file;
	bool valid;
} reader_t;

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

int read_u8(const reader_t *reader, uint8_t *value) {
	return fread(value, sizeof(uint8_t), 1, reader->file) == 1;
}

int read_u16(const reader_t *reader, uint16_t *value) {
	return fread(value, sizeof(uint16_t), 1, reader->file) == 1;
}

int read_u32(const reader_t *reader, uint32_t *value) {
	return fread(value, sizeof(uint32_t), 1, reader->file) == 1;
}

int read_i32(const reader_t *reader, int32_t *value) {
	return fread(value, sizeof(int32_t), 1, reader->file) == 1;
}

int read_u64(const reader_t *reader, uint64_t *value) {
	return fread(value, sizeof(uint64_t), 1, reader->file) == 1;
}

int read_bytes(const reader_t *reader, void *data, const size_t len) {
	return fread(data, 1, len, reader->file) == len;
}

// --- Higher level ---

void write_header(const writer_t *writer, const header_t *header) {
	write_bytes(writer, header->magic, MAGIC_LEN);
	write_u32(writer, header->version);
	write_u32(writer, header->crc32_hash);
}

// Change in case I ever decide to make the color_t a larger int
int write_color(const writer_t *writer, const color_t value) {
	return write_u8(writer, (uint8_t)value);
}

void write_sprite(const writer_t *writer, const sprite_t *sprite) {
	write_u32(writer, sprite->flags);
	write_color(writer, sprite->color_key);
}

void write_entity(const writer_t *writer, const entity_t *entity) {
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
	write_bytes(writer, entity->data.string.data, entity->data.string.len);
}

void write_pattern_step(const writer_t *writer, const pattern_step_t *step) {
	write_u8(writer, step->instrument_index);
	write_u8(writer, step->pitch);
	write_u8(writer, step->volume);
}

void write_instrument(const writer_t *writer, const instrument_t *instrument) {
	write_u8(writer, (uint8_t)instrument->waveform);

	write_u8(writer, instrument->attack);
	write_u8(writer, instrument->decay);
	write_u8(writer, instrument->sustain);
	write_u8(writer, instrument->release);
}

void write_pattern(const writer_t *writer, const pattern_t *pattern) {
	for (uint64_t i = 0; i < STEPS_IN_PATTERN; i++) {
		write_pattern_step(writer, &pattern->steps[i]);
	}
	write_u8(writer, pattern->speed);
	write_u8(writer, pattern->volume);
}

void write_arrangement(const writer_t *writer, const arrangement_t *arrangement) {
	write_bytes(writer, arrangement->pattern_indices, PATTERNS_IN_ARRANGEMENT * sizeof(uint16_t));
}

void game_save(const computer_t *computer, const string_t path) {
	writer_t writer = writer_open(path);

	// Header
	header_t header = {
		.magic = "zinc",
		.version = CURRENT_FILE_VERSION,
		.crc32_hash = 0,
	};
	write_header(&writer, &header);

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
		write_sprite(&writer, &computer->ram->sprites[i]);
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
		write_entity(&writer, &computer->ram->entities.entities[i]);
	}

	// Instruments
	for (uint64_t i = 0; i < MAX_INSTRUMENTS; i++) {
		write_instrument(&writer, &computer->ram->instruments[i]);
	}

	// Patterns
	for (uint64_t i = 0; i < PATTERN_AMOUNT; i++) {
		write_pattern(&writer, &computer->ram->patterns[i]);
	}

	// Arrangements
	for (uint64_t i = 0; i < MAX_ARRANGEMENTS; i++) {
		write_arrangement(&writer, &computer->ram->arrangements[i]);
	}

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
