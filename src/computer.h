/*
Computer

Memory layout, global constants
*/

#pragma once

#include <inttypes.h>
#include <stdbool.h>

#include "core/file.h"
#include "common/math2d.h"
#include "core/input.h"

// Define PI in case that didn't already happen for some reason
#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

// TODO: rename some stuff so it's all consistent, dont mix AMOUNT, MAX, TOTAL etc.

#define RAM_SIZE (32 * 1024 * 1024)
#define CODE_SIZE (8 * 1024 * 1024)

#define SECONDS(x) (x * 60)

#define SCREEN_WIDTH 640
#define SCREEN_HEIGHT 480

#define PALETTE_SIZE 256

// These are in pixels
#define SPRITESHEET_PAGE_WIDTH 256
#define SPRITESHEET_PAGE_HEIGHT 128
#define SPRITESHEET_WIDTH 4 * SPRITESHEET_PAGE_WIDTH
#define SPRITESHEET_HEIGHT 8 * SPRITESHEET_PAGE_HEIGHT

#define SPRITE_WIDTH 8
#define SPRITE_HEIGHT 8
#define SPRITES_PER_ROW (SPRITESHEET_WIDTH / SPRITE_WIDTH)
#define SPRITES_PER_PAGE ((SPRITESHEET_PAGE_WIDTH * SPRITESHEET_PAGE_HEIGHT) / (SPRITE_WIDTH * SPRITE_HEIGHT))
#define TOTAL_SPRITES ((SPRITESHEET_WIDTH * SPRITESHEET_HEIGHT) / (SPRITE_WIDTH * SPRITE_HEIGHT))

#define VISIBLE_CHARACTERS_START 32
#define VISIBLE_CHARACTERS_SIZE 96

#define MAX_CHARACTER_WIDTH 16
#define MAX_CHARACTER_HEIGHT 16

#define COLOR_NONE 255

// First 16 VGA colors
#define COLOR_BLACK 0
#define COLOR_DARKBLUE 1
#define COLOR_DARKGREEN 2
#define COLOR_DARKCYAN 3
#define COLOR_DARKRED 4
#define COLOR_PURPLE 5
#define COLOR_BROWN 6
#define COLOR_LIGHTGRAY 7
#define COLOR_DARKGRAY 8
#define COLOR_BLUE 9
#define COLOR_GREEN 10
#define COLOR_CYAN 11
#define COLOR_RED 12
#define COLOR_PINK 13
#define COLOR_YELLOW 14
#define COLOR_WHITE 15

#define MAP_LAYERS_AMOUNT 4
#define MAP_WIDTH 80 * 16
#define MAP_HEIGHT 60 * 16
#define MAP_LAYER_SIZE (MAP_WIDTH * MAP_HEIGHT)

#define FPS 60
#define FRAME_DELAY 1000/FPS

#define SAMPLE_RATE 44100
#define SAMPLES 256
#define CHANNELS 2
#define MAX_PITCH 48
#define MAX_VOLUME 24
#define MIN_PATTERN_SPEED 1
#define MAX_PATTERN_SPEED 255

#define SKIN_WIDTH SCREEN_WIDTH * 8
#define SKIN_HEIGHT SCREEN_HEIGHT

#define GUI_FONT_INDEX 0
#define CODE_EDITOR_FONT_INDEX 1

#define FILES_AMOUNT 32

#define MAX_ENTITIES 32768

#define BASE_OCTAVE 1

#define MAX_CHANNELS 12
#define MAX_INSTRUMENTS 16
#define MAX_ARRANGEMENTS 128
#define PATTERNS_IN_ARRANGEMENT 6

typedef uint8_t color_t;

typedef struct rgb_color {
	uint8_t r;
	uint8_t g;
	uint8_t b;
} rgb_color_t;

typedef struct palette {
	rgb_color_t colors[PALETTE_SIZE];
} palette_t;

typedef struct framebuffer {
	color_t data[SCREEN_WIDTH * SCREEN_HEIGHT];
} framebuffer_t;

#define SPRITE_FLAGS_SIZE 32
typedef struct sprite {
	uint32_t flags;
	// The last 8 colors are not used anyway so we can use 255 as the value for the sprite not having a color key
	// So we don't need a larger integer
	color_t color_key;
} sprite_t;

// Rename every occurance of sprite_sheet to spritesheet
typedef struct spritesheet {
	color_t data[SPRITESHEET_WIDTH * SPRITESHEET_HEIGHT];
} spritesheet_t;

typedef struct map_layer {
	uint16_t data[MAP_LAYER_SIZE]; // TODO: typedef uint16_t to tile_t
} map_layer_t;

typedef struct map {
	map_layer_t layers[MAP_LAYERS_AMOUNT];
} map_t;

typedef struct font {
	enum {
		SURFACE_SPRITESHEET,
		SURFACE_SKIN,
	} surface;

	uint8_t char_max_width;
	uint8_t char_max_height;

	uint8_t columns;
	point_t start_pos;
	
	uint8_t horizontal_space; // Horizontal space between letters in pixels
	uint8_t vertical_space; // Horizontal space between letters in pixels
	
	uint8_t height; // Height in pixels
	uint8_t widths[VISIBLE_CHARACTERS_SIZE]; // Array of widths in pixels

	color_t color_key;
	color_t seperator_color;
} font_t;

// typedef struct file_collection {
// 	file_t file[32];
// } file_collection_t;

// Should be reset before game is played
// typedef struct draw_state {
// 	int16_t cam_pos_x;
// 	int16_t cam_pos_y;
// } draw_state_t;

typedef struct code_editor_config {
	uint8_t font_index;

	// color_t background_color;
	color_t line_number_color;
	color_t token_colors[LUA_TOKEN_COUNT];
	color_t cursor_color;
	color_t selection_color;
	uint8_t tab_size; // TODO: use this instead of that #define
	uint8_t scroll_speed;
} code_editor_config_t;

typedef struct skin {
	color_t data[SKIN_WIDTH * SKIN_HEIGHT];
	color_t color_key;

	color_t font_color;
} skin_t;

typedef struct {
	uint8_t c;
	color_t bg_color;
	color_t fg_color;
} char_t;

#define TEXTBUFFER_CHAR_WIDTH 8
#define TEXTBUFFER_CHAR_HEIGHT 8
#define TEXTBUFFER_WIDTH 80
#define TEXTBUFFER_HEIGHT 60
#define TEXTBUFFER_SIZE TEXTBUFFER_WIDTH * TEXTBUFFER_HEIGHT
#define TEXT_MODE_FONT_BITMAP_WIDTH 128
#define TEXT_MODE_FONT_BITMAP_HEIGHT 128
#define SHELL_LINE_BUFFER_SIZE 80

typedef struct textbuffer {
	char_t data[TEXTBUFFER_SIZE];
} textbuffer_t;

typedef struct text_mode_font {
	color_t data[TEXT_MODE_FONT_BITMAP_WIDTH * TEXT_MODE_FONT_BITMAP_HEIGHT];
} text_mode_font_t;

// typedef enum video_mode {
// 	VIDEO_MODE_TEXT = 0,
// 	VIDEO_MODE_GRAPHICS = 1,
// } video_mode_t;

typedef struct terminal {
	// uint16_t cursor_index;
	uint8_t cursor_x;
	uint8_t cursor_y;
} terminal_t;

// TODO: make this not part of ram but probably computer
typedef struct shell {
	char line_buffer[SHELL_LINE_BUFFER_SIZE];
	uint8_t line_len;
	string_t_array_t command_history;
	int32_t command_history_index;
} shell_t;

typedef struct entity {
	// uint8_t id[32]; // Unique
	// uint8_t tag[32]; // Not unique, can query
	file_t data;
	
	int32_t x;
	int32_t y;
	
	uint16_t sprite;
	uint8_t w;
	uint8_t h;
	
	uint8_t valid;
} entity_t;

typedef struct entities {
	entity_t entities[MAX_ENTITIES];
	// uint64_t amount;
} entities_t;

typedef enum waveform {
	WAVEFORM_SINE,
	WAVEFORM_SQUARE,
	WAVEFORM_TRIANGLE,
	WAVEFORM_SAWTOOTH,
	WAVEFORM_NOISE,
} waveform_t;

typedef struct instrument {
	waveform_t waveform;

	uint8_t attack;
	uint8_t decay;
	uint8_t sustain; // For some reason this was a float first which it shouldn't have been at all I think
	uint8_t release;
} instrument_t;

typedef struct pattern_step {
	uint8_t instrument_index;
	uint8_t pitch; // Ranges from 0 - 23, so 2 * 12 possibilities or 2 octaves
	uint8_t volume;
} pattern_step_t;

#define STEPS_IN_PATTERN 64
#define PATTERN_AMOUNT 392

typedef struct pattern {
	pattern_step_t steps[STEPS_IN_PATTERN];
	uint8_t speed;
	uint8_t volume;
} pattern_t;

typedef struct arrangement {
	int16_t pattern_indices[PATTERNS_IN_ARRANGEMENT]; // Signed because -1 is used to indicate no pattern
} arrangement_t;

// TODO: Manually align this stuff
typedef union ram {
	struct {
		// VRAM (might wrap that in a struct as well)
		framebuffer_t framebuffer;
		textbuffer_t textbuffer;
		// video_mode_t video_mode;
		palette_t palette;

		spritesheet_t spritesheet;
		sprite_t sprites[TOTAL_SPRITES];
		font_t fonts[8];
		map_t map;
		// draw_state_t draw_state;
		uint64_t ticks;
		code_editor_config_t code_editor_config;
		skin_t skin;
		color_t border_color;
		text_mode_font_t text_mode_font;
		entities_t entities;
		
		pattern_t patterns[PATTERN_AMOUNT];
		instrument_t instruments[MAX_INSTRUMENTS];
		arrangement_t arrangements[MAX_ARRANGEMENTS];

		terminal_t terminal;
		shell_t shell;
	};

	uint8_t data[RAM_SIZE];
} ram_t;

typedef struct sample {
	float left;
	float right;
} sample_t;

typedef enum envelope_stage {
	ENVELOPE_OFF,
	ENVELOPE_ATTACK,
	ENVELOPE_DECAY,
	ENVELOPE_SUSTAIN,
	ENVELOPE_RELEASE,
} envelope_stage_t;

typedef struct channel {
	bool active;

	uint16_t pattern_index;
	int time_left_on_current_step;
	int current_step_index;

	int instrument_index;

	// Voice state
	float phase;
	float frequency;
	float amplitude;
	float env; // amplitude multiplier

	envelope_stage_t envelope_stage;
} channel_t;

typedef enum computer_state {
	// STATE_EDITING,
	// STATE_PLAYING,
	STATE_IN_SHELL,
	STATE_IN_EDITOR,
	STATE_IN_GAME,
} computer_state_t;

typedef struct computer {
	ram_t *ram;
	rgb_color_t rgb_framebuffer[SCREEN_WIDTH * SCREEN_HEIGHT];
	computer_state_t state;
	bool game_running;

	// file_t file;
	file_t files[FILES_AMOUNT];
	uint64_t active_files_amount;

	// TODO: use this
	// file_collection_t file_collection;

	channel_t channels[MAX_CHANNELS];

	string_t current_path;
	string_t game_path; // Absolute path
} computer_t;

// GUI related stuff
typedef struct button {
	rect_t unpressed_rect;
	rect_t pressed_rect;
} button_t;

typedef struct knob {
	int radius;
	point_t text_offset; // from center
} knob_t;

typedef struct button_array {
	button_t base;
	point_t increase; // Used for both positioning in the skin and the program layout, so the skin and layout should match
	int amount;
} button_array_t;

typedef struct button_matrix {
	button_t base;
	int rows;
	int columns;
	int row_increase;
	int column_increase;

	int v_break_size;
	int v_break;

	int h_break_size;
	int h_break;
} button_matrix_t;

void set_global_computer(computer_t *computer);

computer_t *get_global_computer();

void computer_load_resouces(computer_t *computer);

// This function currently only allocates memory for the fantasy ram
void computer_init(computer_t *computer);

void computer_quit(computer_t *computer);

// Play the currently loaded game
void play_game(computer_t *computer);

// Quit the currently loaded game
void quit_game(computer_t *computer);

// Like quit_game, but also puts you in the shell to view the error
// Should only be used when the game is involuntarily quit (because of an error)
void abort_game(computer_t *computer);

/*
int sprite_x_to_sprite_sheet_x(ram_t *ram, int sprite_sheet_index, int sprite_index, int x);

int sprite_y_to_sprite_sheet_y(ram_t *ram, int sprite_sheet_index, int sprite_index, int y);

int sprite_get_pixel(ram_t *ram, int sprite_sheet_index, int sprite_index, int x, int y);

void sprite_set_pixel(ram_t *ram, int sprite_sheet_index, int sprite_index, int x, int y, uint8_t color);
*/

void set_game_path(computer_t *computer, string_t new_path);

void import_file(computer_t *computer, string_t path);
