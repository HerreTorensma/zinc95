/*
Graphics
*/

#pragma once

#include "../computer.h"
#include "math2d.h"

// Generate a buffer of rgb_color_t using palette so it can be rendered by a backend later
void gfx_generate_rgb_framebuffer(computer_t *computer);

void gfx_set_pixel(framebuffer_t *fb, int x, int y, int color);

color_t gfx_get_pixel(framebuffer_t *fb, int x, int y);

void gfx_clear(framebuffer_t *fb, color_t color);

void gfx_draw_rect(framebuffer_t *fb, recti_t rect, color_t color);

void gfx_draw_filled_rect(framebuffer_t *fb, recti_t rect, color_t color);

void gfx_draw_line(framebuffer_t *fb, vec2i_t start, vec2i_t end, color_t color);

void gfx_draw_circle(framebuffer_t *fb, vec2i_t pos, int radius, color_t color);

color_t gfx_spritesheet_get_pixel(spritesheet_t *spritesheet, vec2i_t point);

void gfx_spritesheet_set_pixel(spritesheet_t *spritesheet, vec2i_t point, color_t color);

// TODO: put gfx prefix
recti_t sprite_index_to_spritesheet_rect(int sprite_index, int w, int h);

// 4 ways to draw sprites
// Draw portion of spritesheet at position
void gfx_draw_spritesheet_rect(ram_t *ram, vec2i_t pos, recti_t rect, color_t color_key);

// Draw portion of spritesheet and stretch it to the destination rect
void gfx_draw_spritesheet_pro(ram_t *ram, recti_t source_rect, recti_t dest_rect, color_t color_key);

// Draw a number of sprites by index
void gfx_draw_sprites(ram_t *ram, int index, vec2i_t pos, int width, int height);

// Draw a number of sprites by page index and sprite index
void gfx_draw_sprites_page(ram_t *ram, int page_index, int relative_index, vec2i_t pos, int width, int height);

// Draw sprite, but only in the specified color
// TODO: add to source file and fill in
void gfx_draw_sprite_mask(ram_t *ram, int index, color_t color_key, color_t drawn_color);