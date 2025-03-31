#pragma once

#define BACKEND_SDL2

#include <inttypes.h>
#include <stdbool.h>

#define RAM_SIZE (32 * 1024 * 1024)
#define CODE_SIZE (8 * 1024 * 1024)

#define SCREEN_WIDTH 640
#define SCREEN_HEIGHT 480

#define PALETTE_SIZE 256

// These are in pixels
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

#define VISIBLE_CHARACTERS_SIZE 96
#define VISIBLE_CHARACTERS_START 32
#define MAX_CHARACTER_WIDTH 16
#define MAX_CHARACTER_HEIGHT 16

#define TAB_SIZE 4

#define COLOR_NONE 255
#define COLOR_BLACK 0
#define COLOR_WHITE 15

#define FPS 60

typedef struct rect {
	int x;
	int y;
	int w;
	int h;
} rect_t;

typedef struct rgb_color {
	uint8_t r;
	uint8_t g;
	uint8_t b;
} rgb_color_t;

typedef struct palette {
	rgb_color_t colors[PALETTE_SIZE];
} palette_t;

typedef struct framebuffer {
	uint8_t data[SCREEN_WIDTH * SCREEN_HEIGHT];
} framebuffer_t;

#define SPRITE_FLAGS_SIZE 32
typedef struct sprite {
	uint32_t flags;
	// Need negatives so that's why it's int16_t and not uint8_t
	// NO, the last 8 colors are not used anyway so we can use 255 as not having a key
	uint8_t color_key;

	uint64_t buffer_image;
} sprite_t;

// Rename every occurance of sprite_sheet to spritesheet
typedef struct spritesheet {
	uint8_t data[SPRITESHEET_WIDTH * SPRITESHEET_HEIGHT];
} spritesheet_t;

typedef struct font {
	// Index of first visible ASCII character, which is space ( )
	uint16_t sprite_index;

	uint8_t width;
	uint8_t height;
	uint8_t h_sprites;
	uint8_t v_sprites;
	uint8_t horizontal_space;
	uint8_t vertical_space;

	// Bool
	uint8_t monospace;
	uint8_t widths[VISIBLE_CHARACTERS_SIZE];
} font_t;

typedef struct line {
	char *text;
} line_t;

typedef struct code {
	line_t *lines;
	int line_amount;
	
	int cursor_line;
	int cursor_pos;
	int target_pos;
} code_t;

// 8MB RAM (excluding what the lua code takes up)
typedef union ram {
	struct {
		framebuffer_t framebuffer;
		palette_t palette;
		spritesheet_t spritesheet;
		sprite_t sprites[TOTAL_SPRITES];
		font_t fonts[8];
		char code_buffer[1024 * 1024];
	};

	uint8_t data[RAM_SIZE];
} ram_t;

typedef enum computer_state {
	STATE_EDITING,
	STATE_PLAYING,
} computer_state_t;

typedef struct computer {
	rgb_color_t rgb_framebuffer[SCREEN_WIDTH * SCREEN_HEIGHT];
	ram_t *ram;
	computer_state_t state;
	code_t code;
	uint64_t ticks;
} computer_t;

void set_global_computer(computer_t *computer);

computer_t *get_global_computer();

void computer_load_assets(computer_t *computer);

// This function currently only allocates memory for the fantasy ram
void computer_init(computer_t *computer);

void computer_quit(computer_t *computer);

// Generate a buffer of rgb_color_t using palette so it can be rendered by a backend later
void generate_rgb_framebuffer(computer_t *computer);

// Check if a given point is within the bounds of the framebuffer
bool point_in_bounds(int x, int y);

// Set a pixel in the framebuffer
void set_pixel(computer_t *computer, int x, int y, int color);

// Check if an x, y coordinate is inside a given rect
bool point_in_rect(int x, int y, rect_t rect);

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

void draw_sprite_sheet_rect(computer_t *computer, int x, int y, rect_t rect, uint8_t color_key);

void draw_sprite_sheet_rect_scaled(computer_t *computer, int x, int y, rect_t rect, uint8_t color_key, int scale);

void draw_filled_rectangle(computer_t *computer, rect_t rect, uint8_t color);

rect_t sprite_index_to_spritesheet_rect(ram_t *ram, int sprite_index, int w, int h);

int get_text_width(font_t *font, char text[], int max_offset);

int x_to_text_index(font_t *font, char text[], int x);

void game_save(computer_t *computer, const char filename[]);

void game_load(computer_t *computer, const char filename[]);