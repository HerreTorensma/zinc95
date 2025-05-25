#include "api.h"

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>

#include "../backend/input.h"
#include "../backend/gfx.h"
#include "../backend/gui.h"

const api_meta_t api_metas[API_FUNC_COUNT] = {
	[API_FUNC_CLS] = {
		.name = "cls",
		.signature = "cls(color=0)",
		.desc = "Clear the screen",
	},
	[API_FUNC_RECT] = {
		.name = "rect",
		.signature = "rect(x, y, w, h, color)",
		.desc = "Draw an unfilled rectangle",
	},
	[API_FUNC_RECTF] = {
		.name = "rectf",
		.signature = "rectf(x, y, w, h, color)",
		.desc = "Draw a filled rectangle",
	},
	[API_FUNC_LINE] = {
		.name = "line",
		.signature = "line(x1, y1, x2, y2, color)",
		.desc = "Draw a line from (x1, y1) to (x2, y2)",
	},
	[API_FUNC_CIRC] = {
		.name = "circ",
		.signature = "circ(x, y, radius, color)",
		.desc = "Draw an unfilled circle",
	},
	[API_FUNC_SPR] = {
		.name = "spr",
		.signature = "spr(idx, x, y, [width], [height])",
		.desc = "Draw a sprite by global index",
	},

	[API_FUNC_MAP] = {
		.name = "map",
		.signature = "map(layer, x, y, cell_x, cell_y, cell_w, cell_h)",
		.desc = "Draw a portion of the given map layer",
	},

	[API_FUNC_KEY] = {
		.name = "key",
		.signature = "key(key)",
		.desc = "Key if a key is being held",
	},

	[API_FUNC_TICKS] = {
		.name = "ticks",
		.signature = "ticks()",
		.desc = "Get the amount of ticks the program has been running",
	},
};

void api_meta_print() {
	for (int i = 0; i < API_FUNC_COUNT; i++) {
		printf("### %s\n", api_metas[i].name);
		printf("`%s`\n", api_metas[i].signature);
		printf("%s\n", api_metas[i].desc);
		printf("\n");
	}
}



// TODO: all api functions should have a ram_t *ram argument instead of computer, since only the ram is meant to be modified by the api anyway



void api_cls(ram_t *ram, int color) {
	gfx_clear(FB_SURF(ram->framebuffer.data), color);
}

void api_rect(ram_t *ram, int x, int y, int w, int h, int color) {
	gfx_draw_rect(FB_SURF(ram->framebuffer.data), RECT(x, y, w, h), color);
}

void api_rectf(ram_t *ram, int x, int y, int w, int h, int color) {
	gfx_draw_filled_rect(FB_SURF(ram->framebuffer.data), RECT(x, y, w, h), color);
}

void api_line(ram_t *ram, int x1, int y1, int x2, int y2, int color) {
	gfx_draw_line(FB_SURF(ram->framebuffer.data), POINT(x1, y1), POINT(x2, y2), color);
}

void api_circ(ram_t *ram, int x, int y, int radius, int color) {
	gfx_draw_circle(FB_SURF(ram->framebuffer.data), POINT(x, y), radius, color);
}

void api_spr(ram_t *ram, int idx, int x, int y, int width, int height) {
	gfx_draw_sprites(ram, idx, POINT(x, y), width, height);
}

void api_sspr(ram_t *ram, int dst_x, int dst_y, int dst_w, int dst_h, int src_x, int src_y, int src_w, int src_h, int color_key){
	gfx_draw_spritesheet_pro(ram, RECT(src_x, src_y, src_w, src_h), RECT(dst_x, dst_y, dst_w, dst_h), color_key);
}

void api_map(ram_t *ram, int layer, int x, int y, int cell_x, int cell_y, int cell_w, int cell_h) {
	gfx_draw_map(ram, layer, POINT(x, y), RECT(cell_x, cell_y, cell_w, cell_h));
}



bool api_key(ram_t *ram, int key) {
	return input_key_held(key);
}

bool api_keyp(ram_t *ram, int key) {
	return input_key_pressed(key);
}

bool api_keyr(ram_t *ram, int key) {
	return input_key_released(key);
}

bool api_mouse_btn(ram_t *ram, int button) {
	return input_mouse_button_held(button);
}

bool api_mouse_btnp(ram_t *ram, int button) {
	return input_mouse_button_pressed(button);
}

bool api_mouse_btnr(ram_t *ram, int button) {
	return input_mouse_button_released(button);
}

bool api_mouse_scrolled(ram_t *ram, int direction) {
	return input_mouse_scrolled(direction);
}



void api_text(ram_t *ram, int font_index, char text[], int x, int y, int color) {
	gui_draw_text(ram, font_index, text, POINT(x, y), color);
}



int api_ticks(ram_t *ram) {
	return ram->ticks;
}