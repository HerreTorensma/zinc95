#include "shared.h"

#include <stdio.h>
#include <math.h>

#include "../api/api.h"
#include "../backend/input.h"
#include "../backend/gfx.h"
#include "../backend/gui.h"


static rect_t _page_rect = {0};
static rect_t _in_frame_rect = {0}; // Relative to the page rect

static int _page_index = 0;

static void _set_page_index(int index) {
	_page_index = index;
	_page_rect = (rect_t){
		.x = 0,
		.y = _page_index * SPRITESHEET_PAGE_HEIGHT,
		.w = SPRITESHEET_PAGE_WIDTH,
		.h = SPRITESHEET_PAGE_HEIGHT,
	};
}

void sprite_selector_init(computer_t *computer) {
	_set_page_index(0);
	
	_in_frame_rect = (rect_t){
		.x = 0,
		.y = 0,
		.w = SPRITE_WIDTH,
		.h = SPRITE_HEIGHT,
	};
}

// This whole function is kind of a mess and I should probably rewrite it at some point
void sprite_selector_update(computer_t *computer, sprite_select_snap_mode_t snap_mode, point_t pos) {
	point_t mouse_pos = input_get_mouse_pos();

	if (point_in_rect(mouse_pos, RECT(pos.x, pos.y, SPRITESHEET_PAGE_WIDTH, SPRITESHEET_PAGE_HEIGHT))) {
		if (input_key_pressed(KEY_MINUS) || input_mouse_scrolled(SCROLL_DIR_UP)) {
			_in_frame_rect.w -= SPRITE_WIDTH;
			_in_frame_rect.h -= SPRITE_HEIGHT;
			
			if (_in_frame_rect.w <= 0) {
				_in_frame_rect.w = SPRITE_WIDTH;
			}
			if (_in_frame_rect.h <= 0) {
				_in_frame_rect.h = SPRITE_HEIGHT;
			}
		}

		if (input_key_pressed(KEY_EQUALS) || input_mouse_scrolled(SCROLL_DIR_DOWN)) {
			_in_frame_rect.w += SPRITE_WIDTH;
			_in_frame_rect.h += SPRITE_HEIGHT;
		}

		if (input_mouse_button_held(MOUSE_BUTTON_LEFT)) {
			if (snap_mode == SNAP_MODE_SPRITE) {
				int adjusted_position_x = mouse_pos.x - pos.x;
				int adjusted_position_y = mouse_pos.y - pos.y;

				_in_frame_rect.x = (adjusted_position_x / SPRITE_WIDTH) * SPRITE_WIDTH;
				_in_frame_rect.y = (adjusted_position_y / SPRITE_HEIGHT) * SPRITE_HEIGHT;
			}
			
			else if (snap_mode == SNAP_MODE_FREE) {
				// Free movement

				// TODO: update currently_editing_sprites_rect
				_in_frame_rect.x = mouse_pos.x - pos.x;
				_in_frame_rect.y = mouse_pos.y - pos.y;
			}
			
			else if (snap_mode == SNAP_MODE_ZOOM) {
				int adjusted_position_x = mouse_pos.x - pos.x;
				int adjusted_position_y = mouse_pos.y - pos.y;

				int cell_x = adjusted_position_x / _in_frame_rect.w;
				int cell_y = adjusted_position_y / _in_frame_rect.h;
	
				_in_frame_rect.x = cell_x * _in_frame_rect.w;
				_in_frame_rect.y = cell_y * _in_frame_rect.h;
			}
		}
	}

	// Snap
	if (snap_mode == SNAP_MODE_ZOOM) {
		_in_frame_rect.x = (_in_frame_rect.x / _in_frame_rect.w) * _in_frame_rect.w;
		_in_frame_rect.y = (_in_frame_rect.y / _in_frame_rect.h) * _in_frame_rect.h;

	}
}

void sprite_selector_draw(computer_t *computer, point_t pos, point_t page_buttons_pos) {
	gfx_draw_spritesheet_rect(computer->ram, pos, _page_rect, COLOR_NONE);

	for (int i = 0; i < skin_layout.spritesheet_page_buttons.amount; i++) {
		point_t pos = button_array_get_pos(&skin_layout.spritesheet_page_buttons, page_buttons_pos, i);
		button_t button = button_array_get(&skin_layout.spritesheet_page_buttons, i);

		if (gui_button(computer->ram, pos, button, _page_index == i)) {
			_set_page_index(i);
		}
	}

	gfx_draw_rect(FB_SURF(computer->ram->framebuffer.data), RECT(pos.x + _in_frame_rect.x - 1, pos.y + _in_frame_rect.y - 1, _in_frame_rect.w + 2, _in_frame_rect.h + 2), 15);
}

rect_t get_page_rect() {
	return _page_rect;
}

rect_t get_in_frame_rect() {
	return _in_frame_rect;
}

rect_t get_in_frame_rect_in_sprites() {
	return (rect_t) {
		// .x = (_selected_rect.x / _selected_rect.w) * _selected_rect.w,
		// .y = (_selected_rect.y / _selected_rect.h) * _selected_rect.h,
		.x = _in_frame_rect.x / SPRITE_WIDTH,
		.y = _in_frame_rect.y / SPRITE_HEIGHT,
		.w = _in_frame_rect.w / SPRITE_WIDTH,
		.h = _in_frame_rect.h / SPRITE_HEIGHT,
	};
}

int get_page_index() {
	return _page_index;
}

int get_relative_sprite_index() {
	rect_t rect = get_in_frame_rect_in_sprites();

	return rect.y * (SPRITESHEET_WIDTH / SPRITE_WIDTH) + rect.x;
}

int get_absolute_sprite_index() {
	return (_page_index * SPRITES_PER_PAGE) + get_relative_sprite_index();
}

int sprite_coords_to_index(int x, int y) {
	return y * (SPRITESHEET_WIDTH / SPRITE_WIDTH) + x;
}

point_t editor_to_spritesheet_pos(point_t point) {
	return POINT(_page_rect.x + _in_frame_rect.x + point.x, _page_rect.y + _in_frame_rect.y + point.y);
}
