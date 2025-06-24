/*
Computer

Memory layout, global constants
*/

#pragma once

#define BACKEND_SDL2

#include <inttypes.h>
#include <stdbool.h>

#include "backend/text_file.h"
#include "backend/math2d.h"

// TODO: rename some stuff so it's all consistent, dont mix AMOUNT, MAX, TOTAL etc.

#define RAM_SIZE (32 * 1024 * 1024)
#define CODE_SIZE (8 * 1024 * 1024)

#define SCREEN_WIDTH 640
#define SCREEN_HEIGHT 480

#define PALETTE_SIZE 256

// These are in pixels
// TODO: put them in terms of sprite width and height
#define SPRITESHEET_PAGE_WIDTH 384
#define SPRITESHEET_PAGE_HEIGHT 128
#define SPRITESHEET_WIDTH SPRITESHEET_PAGE_WIDTH
// 8 pages of sprites
// #define SPRITESHEET_PAGE_AMOUNT 16
#define SPRITESHEET_PAGE_AMOUNT 8
#define SPRITESHEET_HEIGHT (SPRITESHEET_PAGE_HEIGHT * SPRITESHEET_PAGE_AMOUNT)

#define SPRITE_WIDTH 8
#define SPRITE_HEIGHT 8
#define SPRITES_PER_ROW (SPRITESHEET_WIDTH / SPRITE_WIDTH)
#define SPRITES_PER_PAGE ((SPRITESHEET_PAGE_WIDTH * SPRITESHEET_PAGE_HEIGHT) / (SPRITE_WIDTH * SPRITE_HEIGHT))
#define TOTAL_SPRITES ((SPRITESHEET_WIDTH * SPRITESHEET_HEIGHT) / (SPRITE_WIDTH * SPRITE_HEIGHT))

#define VISIBLE_CHARACTERS_START 32
#define VISIBLE_CHARACTERS_SIZE 96

#define MAX_CHARACTER_WIDTH 16
#define MAX_CHARACTER_HEIGHT 16

// TODO: use ram code editor config value instead
#define TAB_SIZE 2

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
#define SAMPLES 1024
#define CHANNELS 2

#define SKIN_WIDTH SCREEN_WIDTH * 5
#define SKIN_HEIGHT SCREEN_HEIGHT

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
	uint16_t data[MAP_LAYER_SIZE];
} map_layer_t;

typedef struct map {
	map_layer_t layers[MAP_LAYERS_AMOUNT];
} map_t;

typedef struct font {
	// Index of first visible ASCII character, which is space ( )
	uint16_t sprite_index;

	uint8_t sprite_width; // Width in sprites
	uint8_t sprite_height; // Height in sprites
	
	uint8_t horizontal_space; // Horizontal space between letters in pixels
	uint8_t vertical_space; // Horizontal space between letters in pixels
	
	uint8_t height; // Height in pixels
	uint8_t widths[VISIBLE_CHARACTERS_SIZE]; // Array of widths in pixels
} font_t;

typedef struct file_collection {
	file_t file[32];
} file_collection_t;

// Should be reset before game is played
// typedef struct draw_state {
// 	int16_t cam_pos_x;
// 	int16_t cam_pos_y;
// } draw_state_t;

// Colors used by GUI
// TODO: remove completely because it's replaced by the skin system
typedef struct gui_colors {
	// Since the borders of the screen are not in the framebuffer it can just be an rgb color
	rgb_color_t screen_background;

	color_t text;
	color_t inset_frame_background;
	color_t outset_frame_background;

	// From dark to bright
	color_t frame_edge_darker;
	color_t frame_edge_dark;
	color_t frame_edge_neutral;
	color_t frame_edge_light;

	color_t toggle_button_set_text;
	color_t toggle_button_unset_text;
} gui_colors_t;

typedef struct code_editor_config {
	uint8_t font_index;

	// color_t background;
	// color_t keyword;
	// color_t string;
	// color_t api_function;
	// color_t operator;
	// color_t other;
	color_t background_color;
	color_t token_colors[LUA_TOKEN_COUNT];
	uint8_t tab_size; // TODO: use this instead of that #define
} code_editor_config_t;

typedef struct skin {
	color_t data[SKIN_WIDTH * SKIN_HEIGHT];
	color_t color_key;

	uint8_t font_index;
	color_t font_color;
} skin_t;

// 8MB RAM (excluding what the lua code takes up)
typedef union ram {
	struct {
		framebuffer_t framebuffer;
		palette_t palette;
		spritesheet_t spritesheet;
		sprite_t sprites[TOTAL_SPRITES];
		font_t fonts[8];
		map_t map;
		// draw_state_t draw_state;
		gui_colors_t gui_colors;
		uint64_t ticks;
		code_editor_config_t code_editor_config;
		skin_t skin;
	};

	uint8_t data[RAM_SIZE];
} ram_t;

typedef enum computer_state {
	STATE_EDITING,
	STATE_PLAYING,
} computer_state_t;

typedef struct sample {
	float left;
	float right;
} sample_t;

typedef enum waveform {
	WAVEFORM_SINE,
	WAVEFORM_SQUARE,
	WAVEFORM_TRIANGLE,
} waveform_t;

typedef struct oscillator {
	waveform_t waveform;
	float freq;
	float phase;
} oscillator_t;

typedef struct voice {
	oscillator_t oscillator;
	float amplitude;

	// float duration;
	// float time;
	
	bool active;
} voice_t;

#define MAX_VOICES 32

typedef struct voice_pool {
	voice_t voices[MAX_VOICES];
} voice_pool_t;

typedef struct computer {
	ram_t *ram;
	rgb_color_t rgb_framebuffer[SCREEN_WIDTH * SCREEN_HEIGHT];
	computer_state_t state;

	file_t file;

	// TODO: use this
	// file_collection_t file_collection;

	// TODO: Currenly stack allocated 1MB,
	// later when I have the file collection system I will make this dynamically allocated
	// I tried that currently but I'll have to revamp it again to why bother
	char code_buffer[1024 * 1024 * sizeof(char)];

	voice_pool_t voice_pool;
} computer_t;

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

/*
int sprite_x_to_sprite_sheet_x(ram_t *ram, int sprite_sheet_index, int sprite_index, int x);

int sprite_y_to_sprite_sheet_y(ram_t *ram, int sprite_sheet_index, int sprite_index, int y);

int sprite_get_pixel(ram_t *ram, int sprite_sheet_index, int sprite_index, int x, int y);

void sprite_set_pixel(ram_t *ram, int sprite_sheet_index, int sprite_index, int x, int y, uint8_t color);
*/

void game_save(computer_t *computer, const char filename[]);

void game_load(computer_t *computer, const char filename[]);

// GUI related stuff
typedef struct button {
	rect_t unpressed_rect;
	rect_t pressed_rect;
} button_t;

typedef struct button_array {
	button_t base;
	point_t increase; // Used for both positioning in the skin and the program layout, so the skin and layout should match
	int amount;
} button_array_t;

typedef struct skin_layout {
	button_t code_button;
	button_t sprite_button;
	button_t map_button;
	button_t sound_button;

	button_t save_button;
	button_t play_button;

	button_array_t sprite_flag_buttons;

	button_t color_key_button;
	
	button_array_t spritesheet_page_buttons;

	button_t map_entity_layer_button;
	button_array_t map_layer_buttons;

	button_array_t sprite_tool_buttons;
} skin_layout_t;

extern const skin_layout_t skin_layout;

void skin_load(ram_t *ram, const char filename[]);
