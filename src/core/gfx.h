/*
Graphics
*/

#pragma once

#include "../computer.h"
#include "../common/math2d.h"

// TODO: use this in gfx draw functions instead of framebuffer_t,
// so it can later also be used for drawing on the spritesheet
// also for drawing shapes on the spritesheet, I need a second sprite sheet that exists on computer that is overlayed so that you can preview
// what you draw
// or just draw it to the framebuffer actually what am I talking about
typedef struct surface {
	color_t *data;
	int width; // TODO: rename to w and h
	int height;
	
	// TODO: possibly uncomment and use this struct in the map editor for the tile operations
	// int bytes_per_pixel;
} surface_t;

// #define SURF(_data, _width, _height) ((surface_t){.data = _data, .width = _width, .height = _height})

// Macro for the framebuffer surface, so you don't have to manually input the SCREEN_WIDTH and SCREEN_HEIGHT every time
#define FB_SURF(_data) ((surface_t){.data = (color_t *)_data, .width = SCREEN_WIDTH, .height = SCREEN_HEIGHT})

// Same for the spritesheet
#define SPR_SURF(_data) ((surface_t){.data = (color_t *)_data, .width = SPRITESHEET_WIDTH, .height = SPRITESHEET_HEIGHT})

// Skin
#define SKIN_SURF(_data) ((surface_t){.data = (color_t *)_data, .width = SKIN_WIDTH, .height = SKIN_HEIGHT})

// void surf_set_pixel(surface_t surf, int x, int y, int color);

// color_t surf_get_pixel(surface_t surf, int x, int y);

// Generate a buffer of rgb_color_t using palette so it can be rendered by a backend later
void gfx_generate_rgb_framebuffer(computer_t *computer);

color_t gfx_rgb_color_to_color(palette_t *palette, rgb_color_t rgb_color, color_t undefined_color);

static inline bool point_in_bounds(surface_t surf, int x, int y) {
	if (x < 0) return false;
	if (x >= surf.width) return false;
	if (y < 0) return false;
	if (y >= surf.height) return false;

	return true;
}

static inline void surf_set_pixel(surface_t surf, int x, int y, int color) {
	if (point_in_bounds(surf, x, y)) {
		surf.data[y * surf.width + x] = color;
	}
}

static inline color_t surf_get_pixel(surface_t surf, int x, int y) {
	if (point_in_bounds(surf, x, y)) {
		return surf.data[y * surf.width + x];
	}
	return COLOR_NONE;
}

// Check if the given point is on the screen bounds
static inline bool point_in_screen(int x, int y) {
	if (x < 0) return false;
	if (x >= SCREEN_WIDTH) return false;
	if (y < 0) return false;
	if (y >= SCREEN_HEIGHT) return false;
	
	return true;
}

// Set a pixel on the framebuffer
static inline void gfx_set_pixel(framebuffer_t *fb, int x, int y, int color) {
	if (point_in_screen(x, y)) {
		fb->data[y * SCREEN_WIDTH + x] = color;
	}
}

// Get a pixel from the framebuffer
static inline color_t gfx_get_pixel(framebuffer_t *fb, int x, int y) {
	if (point_in_screen(x, y)) {
		return fb->data[y * SCREEN_WIDTH + x];
	}
	return COLOR_NONE;
}

void gfx_clear(surface_t surf, color_t color);

void gfx_draw_rect(surface_t surf, rect_t rect, color_t color);

void gfx_draw_filled_rect(surface_t surf, rect_t rect, color_t color);

void gfx_draw_line(surface_t surf, point_t start, point_t end, color_t color);

void gfx_draw_circle(surface_t surf, point_t pos, int radius, color_t color);

void gfx_draw_ellipse(surface_t surf, rect_t bound, color_t color);

void gfx_draw_filled_ellipse(surface_t surf, rect_t bound, color_t color);

color_t gfx_spritesheet_get_pixel(spritesheet_t *spritesheet, point_t point);

void gfx_spritesheet_set_pixel(spritesheet_t *spritesheet, point_t point, color_t color);

// TODO: put gfx prefix
// But I'm not sure if this should be in this file
rect_t sprite_index_to_spritesheet_rect(int sprite_index, int w, int h);

void gfx_copy_surface_rect(surface_t dest, surface_t src, point_t dest_pos, rect_t source_rect, color_t color_key);

// TODO: maybe remove this function???
void gfx_draw_surface_rect(framebuffer_t *fb, surface_t surf, point_t dest_pos, rect_t source_rect, color_t color_key);

// 4 ways to draw sprites
// Draw portion of spritesheet at position
void gfx_draw_spritesheet_rect(ram_t *ram, point_t pos, rect_t rect, color_t color_key);

// TODO: rename to blit?
void gfx_draw_surface_pro(framebuffer_t *fb, surface_t surf, rect_t source_rect, rect_t dest_rect, color_t color_key, rect_t clip_rect);

// Draw portion of spritesheet and stretch it to the destination rect
void gfx_draw_spritesheet_pro(ram_t *ram, rect_t source_rect, rect_t dest_rect, color_t color_key, rect_t clip_rect);

// Draw a number of sprites by index
void gfx_draw_sprites(ram_t *ram, int index, point_t pos, int width, int height);

// Draw a number of sprites by page index and sprite index
void gfx_draw_sprites_page(ram_t *ram, int page_index, int relative_index, point_t pos, int width, int height);

// Draw sprite, but only in the specified color
// TODO: add to source file and fill in
void gfx_draw_sprite_mask(ram_t *ram, int index, color_t color_key, color_t drawn_color);

// Draw the a map layer
// A mask with all 1's will always draw everything even if the sprite flags are all 0
void gfx_draw_map(ram_t *ram, int layer_index, point_t pos, rect_t section, float scale, color_t color_key, rect_t clip_rect, uint32_t mask);

// TODO: draw map but only the tiles with some flags

// Expects surface to be the same dimension as the image at the filename
// Because it's only really used to load the skin which has a static size
// Only BMP is supported
void gfx_load_surface(palette_t *palette, surface_t surface, string_t path);

void gfx_save_surface(palette_t *palette, surface_t surface, string_t path);

void gfx_flood_fill(surface_t surface, point_t point, color_t color, rect_t limit);

void gfx_draw_string(ram_t *ram, int font_index, string_t string, point_t pos, int color);

void gfx_draw_text(ram_t *ram, int font_index, const char text[], point_t pos, int color);
