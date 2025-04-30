#include "api.h"

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>

#include "../backend/input.h"
#include "../backend/gfx.h"
#include "../backend/gui.h"

// TODO: remove api calls anywhere else in the code
// idk I want it to exist in a bubble I guess



void api_cls(computer_t *computer, int color) {
	gfx_clear(&computer->ram->framebuffer, color);
}

void api_rect(computer_t *computer, int x, int y, int w, int h, int color) {
	gfx_draw_rect(&computer->ram->framebuffer, RECT(x, y, w, h), color);
}

void api_rectf(computer_t *computer, int x, int y, int w, int h, int color) {
	gfx_draw_filled_rect(&computer->ram->framebuffer, RECT(x, y, w, h), color);
}

void api_line(computer_t *computer, int x1, int y1, int x2, int y2, int color) {
	gfx_draw_line(&computer->ram->framebuffer, POINT(x1, y1), POINT(x2, y2), color);
}

void api_circ(computer_t *computer, int x, int y, int radius, int color) {
	gfx_draw_circle(&computer->ram->framebuffer, POINT(x, y), radius, color);
}

void api_spr(computer_t *computer, int sprite_index, int x, int y, int width, int height) {
	gfx_draw_sprites(computer->ram, sprite_index, POINT(x, y), width, height);
}

void api_sspr(computer_t *computer, int dst_x, int dst_y, int dst_w, int dst_h, int src_x, int src_y, int src_w, int src_h, int color_key){
	gfx_draw_spritesheet_pro(computer->ram, RECT(src_x, src_y, src_w, src_h), RECT(dst_x, dst_y, dst_w, dst_h), color_key);
}

void api_draw_map_layer(computer_t *computer, int layer, int x, int y, int cell_x, int cell_y, int cell_w, int cell_h) {
	gfx_draw_map(computer->ram, layer, POINT(x, y), RECT(cell_x, cell_y, cell_w, cell_h));
}



bool api_key(computer_t *computer, int key) {
	return input_key_held(key);
}

bool api_keyp(computer_t *computer, int key) {
	return input_key_pressed(key);
}

bool api_keyr(computer_t *computer, int key) {
	return input_key_released(key);
}

bool api_mouse_btn(computer_t *computer, int button) {
	return input_mouse_button_held(button);
}

bool api_mouse_btnp(computer_t *computer, int button) {
	return input_mouse_button_pressed(button);
}

bool api_mouse_btnr(computer_t *computer, int button) {
	return input_mouse_button_released(button);
}

bool api_mouse_scrolled(computer_t *computer, int direction) {
	return input_mouse_scrolled(direction);
}



void api_text(computer_t *computer, int font_index, char text[], int x, int y, int color) {
	gui_draw_text(computer->ram, font_index, text, POINT(x, y), color);
}



int api_ticks(computer_t *computer) {
	return computer->ram->ticks;
}