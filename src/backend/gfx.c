#include "gfx.h"

#include <stdlib.h>

static bool point_in_screen(int x, int y) {
	if (x < 0) return false;
	if (x >= SCREEN_WIDTH) return false;
	if (y < 0) return false;
	if (y >= SCREEN_HEIGHT) return false;

	return true;
}

void gfx_generate_rgb_framebuffer(computer_t *computer) {
	for (int y = 0; y < SCREEN_HEIGHT; y++) {
		for (int x = 0; x < SCREEN_WIDTH; x++) {
			uint8_t pixel = computer->ram->framebuffer.data[y * SCREEN_WIDTH + x];
			computer->rgb_framebuffer[y * SCREEN_WIDTH + x] = computer->ram->palette.colors[pixel];
		}
	}
}


void gfx_set_pixel(framebuffer_t *fb, int x, int y, int color) {
	if (point_in_screen(x, y)) {
		fb->data[y * SCREEN_WIDTH + x] = color;
	}
}

color_t gfx_get_pixel(framebuffer_t *fb, int x, int y) {
	if (point_in_screen(x, y)) {
		return fb->data[y * SCREEN_WIDTH + x];
	}
	return COLOR_NONE;
}

void gfx_clear(framebuffer_t *fb, color_t color) {
	for (int y = 0; y < SCREEN_HEIGHT; y++) {
		for (int x = 0; x < SCREEN_WIDTH; x++) {
			gfx_set_pixel(fb, x, y, color);
		}
	}
}

// TODO: make these geometry functions take any uint8_t array and a vec2i of size
// so they can be used both for the framebuffer and the spritesheet
void gfx_draw_rect(framebuffer_t *fb, rect_t rect, color_t color) {
	for (int j = rect.x; j < rect.x+rect.w; j++) {
		gfx_set_pixel(fb, j, rect.y, color);
	}

	for (int j = rect.x; j < rect.x+rect.w; j++) {
		gfx_set_pixel(fb, j, rect.y+rect.h-1, color);
	}

	for (int i = rect.y; i < rect.y+rect.h; i++) {
		gfx_set_pixel(fb, rect.x, i, color);
	}

	for (int i = rect.y; i < rect.y+rect.h; i++) {
		gfx_set_pixel(fb, rect.x+rect.w - 1, i, color);
	}
}

void gfx_draw_filled_rect(framebuffer_t *fb, rect_t rect, color_t color) {
	for (int i = rect.y; i < rect.y+rect.h; i++) {
		for (int j = rect.x; j < rect.x+rect.w; j++) {
			gfx_set_pixel(fb, j, i, color);
		}
	}
}

// Using Bresemham's line algorithm
void gfx_draw_line(framebuffer_t *fb, point_t start, point_t end, color_t color) {
	int x1 = start.x;
	int y1 = start.y;
	int x2 = end.x;
	int y2 = end.y;
	
	int dx = abs(x2 - x1);
	int dy = abs(y2 - y1);
	int step_x = (x1 < x2) ? 1 : -1;
	int step_y = (y1 < y2) ? 1 : -1;
	int error = dx - dy;
	
	while (true) {
		gfx_set_pixel(fb, x1, y1, color);
		
		if (x1 == x2 && y1 == y2) break;
		
		int e2 = 2 * error;
		if (e2 > -dy) {
			error -= dy;
			x1 += step_x;
		}
		if (e2 < dx) {
			error += dx;
			y1 += step_y;
		}
	}
}

// Midpoint circle algorithm
// TODO: adopt for ellipses
void gfx_draw_circle(framebuffer_t *fb, point_t pos, int radius, color_t color) {
	int x = pos.x;
	int y = pos.y;
	
	// Initial 4 points
	gfx_set_pixel(fb, x + radius, y, color);
	gfx_set_pixel(fb, x - radius, y, color);
	gfx_set_pixel(fb, x, y + radius, color);
	gfx_set_pixel(fb, x, y - radius, color);

	int x_offset = radius;
	int y_offset = 0;

	int d = 1 - radius;

	while (x_offset > y_offset) {
		y_offset++;
		
		if (d <= 0) {
			d = d + 2 * y_offset + 1;
		} else {
			x_offset--;
			d = d + 2 * (y_offset - x_offset) + 1;
		}
		
		if (x_offset < y_offset) {
			break;
		}

		gfx_set_pixel(fb, x + x_offset, y + y_offset, color);
		gfx_set_pixel(fb, x - x_offset, y + y_offset, color);
		gfx_set_pixel(fb, x + x_offset, y - y_offset, color);
		gfx_set_pixel(fb, x - x_offset, y - y_offset, color);

		if (x_offset != y_offset) {
			gfx_set_pixel(fb, x + y_offset, y + x_offset, color);
			gfx_set_pixel(fb, x - y_offset, y + x_offset, color);
			gfx_set_pixel(fb, x + y_offset, y - x_offset, color);
			gfx_set_pixel(fb, x - y_offset, y - x_offset, color);
		}
	}
}

color_t gfx_spritesheet_get_pixel(spritesheet_t *spritesheet, point_t point) {
	// TODO: clip the wanted rect instead of this check ???
	if (point_in_rect(point, RECT(0, 0, SPRITESHEET_WIDTH, SPRITESHEET_HEIGHT))) {
		return spritesheet->data[point.y * SPRITESHEET_WIDTH + point.x];
	}
	// TODO: should this be 0 or COLOR_NONE ???
	return COLOR_NONE;
}

void gfx_spritesheet_set_pixel(spritesheet_t *spritesheet, point_t point, color_t color) {
	if (point_in_rect(point, RECT(0, 0, SPRITESHEET_WIDTH, SPRITESHEET_HEIGHT))) {
		spritesheet->data[point.y * SPRITESHEET_WIDTH + point.x] = color;
	}
}

rect_t sprite_index_to_spritesheet_rect(int sprite_index, int w, int h) {
	rect_t rect = {
		.x = (sprite_index % SPRITES_PER_ROW) * SPRITE_WIDTH,
		.y = (sprite_index / SPRITES_PER_ROW) * SPRITE_HEIGHT,
		.w = w * SPRITE_WIDTH,
		.h = h * SPRITE_HEIGHT,
	};

	return rect;
}

void gfx_draw_spritesheet_rect(ram_t *ram, point_t pos, rect_t rect, color_t color_key) {
	for (int i = 0; i < rect.h; i++) {
		for (int j = 0; j < rect.w; j++) {
			uint8_t color = gfx_spritesheet_get_pixel(&ram->spritesheet, (point_t){rect.x + j, rect.y + i});
			if (color != color_key) {
				gfx_set_pixel(&ram->framebuffer, pos.x + j, pos.y + i, color);
			}
		}
	}
}

void gfx_draw_spritesheet_pro(ram_t *ram, rect_t source_rect, rect_t dest_rect, color_t color_key) {
	for (int y = 0; y < dest_rect.h; y++) {
		for (int x = 0; x < dest_rect.w; x++) {
			// Calculate normalized coords
			float u = (float)x / (float)dest_rect.w;
			float v = (float)y / (float)dest_rect.h;

			// Then convert to source coords
			int source_x = source_rect.x + u * source_rect.w;
			int source_y = source_rect.y + v * source_rect.h;

			uint8_t color = gfx_spritesheet_get_pixel(&ram->spritesheet, (point_t){source_x, source_y});
			
			if (color != color_key) {
				gfx_set_pixel(&ram->framebuffer, dest_rect.x + x, dest_rect.y + y, color);
			}
		}
	}
}

// TODO: implement flip_x, flip_y
void gfx_draw_sprites(ram_t *ram, int index, point_t pos, int width, int height) {
	rect_t rect = sprite_index_to_spritesheet_rect(index, width, height);
	
	gfx_draw_spritesheet_rect(ram, pos, rect, ram->sprites[index].color_key);
}

void gfx_draw_sprites_page(ram_t *ram, int page_index, int relative_index, point_t pos, int width, int height) {
	int absolute_index = page_index * SPRITES_PER_PAGE + relative_index;

	rect_t rect = sprite_index_to_spritesheet_rect(absolute_index, width, height);

	gfx_draw_spritesheet_rect(ram, pos, rect, ram->sprites[absolute_index].color_key);
}

void gfx_draw_map(ram_t *ram, int layer_index, point_t pos, rect_t section) {
	section = rect_clip(RECT(0, 0, MAP_WIDTH, MAP_HEIGHT), section);

	for (int i = section.y; i < section.y + section.h; i++) {
		for (int j = section.x; j < section.x + section.w; j++) {
			int sprite_index = ram->map.layers[layer_index].data[i * MAP_WIDTH + j];
			gfx_draw_sprites(ram, sprite_index, POINT(pos.x + j * SPRITE_WIDTH, pos.y + i * SPRITE_HEIGHT), 1, 1);
		}
	}
}