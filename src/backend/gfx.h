/*
Graphics
*/

#pragma once

#include "../computer.h"
#include "math2d.h"

// TODO: use this in gfx draw functions instead of framebuffer_t,
// so it can later also be used for drawing on the spritesheet
// also for drawing shapes on the spritesheet, I need a second sprite sheet that exists on computer that is overlayed so that you can preview
// what you draw
// or just draw it to the framebuffer actually what am I talking about
typedef struct surface {
	color_t *data;
	int width;
	int height;
} surface_t;

// #define SURF(_data, _width, _height) ((surface_t){.data = _data, .width = _width, .height = _height})

// Macro for the framebuffer surface, so you don't have to manually input the SCREEN_WIDTH and SCREEN_HEIGHT every time
#define FB_SURF(_data) ((surface_t){.data = (color_t *)_data, .width = SCREEN_WIDTH, .height = SCREEN_HEIGHT})

// Same for the spritesheet
#define SPR_SURF(_data) ((surface_t){.data = (color_t *)_data, .width = SPRITESHEET_WIDTH, .height = SPRITESHEET_HEIGHT})

void surf_set_pixel(surface_t surf, int x, int y, int color);

color_t surf_get_pixel(surface_t surf, int x, int y);

// Generate a buffer of rgb_color_t using palette so it can be rendered by a backend later
void gfx_generate_rgb_framebuffer(computer_t *computer);

// Set a pixel on the framebuffer
void gfx_set_pixel(framebuffer_t *fb, int x, int y, int color);

// Get a pixel from the framebuffer
color_t gfx_get_pixel(framebuffer_t *fb, int x, int y);

void gfx_clear(surface_t surf, color_t color);

void gfx_draw_rect(surface_t surf, rect_t rect, color_t color);

void gfx_draw_filled_rect(surface_t surf, rect_t rect, color_t color);

void gfx_draw_line(surface_t surf, point_t start, point_t end, color_t color);

void gfx_draw_circle(surface_t surf, point_t pos, int radius, color_t color);

void gfx_flood_fill(surface_t surf, point_t start, color_t color);

color_t gfx_spritesheet_get_pixel(spritesheet_t *spritesheet, point_t point);

void gfx_spritesheet_set_pixel(spritesheet_t *spritesheet, point_t point, color_t color);

// TODO: put gfx prefix
// But I'm not sure if this should be in this file
rect_t sprite_index_to_spritesheet_rect(int sprite_index, int w, int h);

void gfx_copy_surface_rect(surface_t dest, surface_t src, point_t pos, rect_t rect, color_t color_key);

void gfx_draw_surface_rect(framebuffer_t *fb, surface_t surf, point_t pos, rect_t rect, color_t color_key);

// 4 ways to draw sprites
// Draw portion of spritesheet at position
void gfx_draw_spritesheet_rect(ram_t *ram, point_t pos, rect_t rect, color_t color_key);

void gfx_draw_surface_pro(framebuffer_t *fb, surface_t surf, rect_t source_rect, rect_t dest_rect, color_t color_key);

// Draw portion of spritesheet and stretch it to the destination rect
void gfx_draw_spritesheet_pro(ram_t *ram, rect_t source_rect, rect_t dest_rect, color_t color_key);

// Draw a number of sprites by index
void gfx_draw_sprites(ram_t *ram, int index, point_t pos, int width, int height);

// Draw a number of sprites by page index and sprite index
void gfx_draw_sprites_page(ram_t *ram, int page_index, int relative_index, point_t pos, int width, int height);

// Draw sprite, but only in the specified color
// TODO: add to source file and fill in
void gfx_draw_sprite_mask(ram_t *ram, int index, color_t color_key, color_t drawn_color);

void gfx_draw_map(ram_t *ram, int layer_index, point_t pos, rect_t section);

// Load bmp, might just use stb image actually
void load_bmp(surface_t *surface, const char filename[]);