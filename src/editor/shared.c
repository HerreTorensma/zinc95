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

static void _copy_in_frame_sprites() {

}

static void _delete_in_frame_sprites(ram_t *ram) {
	// TODO: add visible rect stuff
	for (int y = _in_frame_rect.y; y < _in_frame_rect.y + _in_frame_rect.h; y++) {
		for (int x = _in_frame_rect.x; x < _in_frame_rect.x + _in_frame_rect.w; x++) {
			ram->spritesheet.data[y * SPRITESHEET_WIDTH + x] = COLOR_BLACK;
		}
	}
	
	rect_t in_frame_rect_in_sprites = get_in_frame_rect_in_sprites();
	for (int y = in_frame_rect_in_sprites.y; y < in_frame_rect_in_sprites.y + in_frame_rect_in_sprites.h; y++) {
		for (int x = in_frame_rect_in_sprites.x; x < in_frame_rect_in_sprites.x + in_frame_rect_in_sprites.w; x++) {
			// TODO: actually clear this stuff for every sprite in the selection
			ram->sprites[get_absolute_sprite_index()].color_key = 0;
			ram->sprites[get_absolute_sprite_index()].flags = 0U;
		}
	}
}

// This whole function is kind of a mess and I should probably rewrite it at some point
void sprite_selector_update(computer_t *computer, sprite_select_snap_mode_t snap_mode, point_t pos) {
	point_t mouse_pos = input_get_mouse_pos();

	if (point_in_rect(mouse_pos, RECT(pos.x, pos.y, SPRITESHEET_PAGE_WIDTH, SPRITESHEET_PAGE_HEIGHT))) {
		if (input_key_pressed(KEY_MINUS) || input_mouse_scrolled(SCROLL_DIR_UP)) {
			/*
			_in_frame_rect.w -= SPRITE_WIDTH;
			_in_frame_rect.h -= SPRITE_HEIGHT;

			if (_in_frame_rect.w <= 0) {
				_in_frame_rect.w = SPRITE_WIDTH;
			}
			if (_in_frame_rect.h <= 0) {
				_in_frame_rect.h = SPRITE_HEIGHT;
			}
			*/

			_in_frame_rect.w /= 2;
			_in_frame_rect.h /= 2;
			
			if (_in_frame_rect.w <= SPRITE_WIDTH || _in_frame_rect.h <= SPRITE_HEIGHT) {
				_in_frame_rect.w = SPRITE_WIDTH;
				_in_frame_rect.h = SPRITE_HEIGHT;
			}
		}

		if (input_key_pressed(KEY_EQUALS) || input_mouse_scrolled(SCROLL_DIR_DOWN)) {
			/*
			_in_frame_rect.w += SPRITE_WIDTH;
			_in_frame_rect.h += SPRITE_HEIGHT;
			*/

			_in_frame_rect.w *= 2;
			_in_frame_rect.h *= 2;

			if (_in_frame_rect.w > SPRITESHEET_WIDTH || _in_frame_rect.h > SPRITESHEET_HEIGHT) {
				_in_frame_rect.w /= 2;
				_in_frame_rect.h /= 2;
			}
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

		if (input_key_held(KEY_LCTRL) && input_key_pressed(KEY_C)) {
			// Copy
		}

		// Delete sprite
		if (input_key_pressed(KEY_DELETE)) {
			_delete_in_frame_sprites(computer->ram);
		}

		if (input_key_held(KEY_LCTRL) && input_key_pressed(KEY_X)) {
			// Copy

			// Delete
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

// The coordinates of _page_rect first needs to be added to account for the pages
rect_t get_in_frame_rect() {
	return (rect_t){
		.x = _page_rect.x + _in_frame_rect.x,
		.y = _page_rect.y + _in_frame_rect.y,
		.w = _in_frame_rect.w,
		.h = _in_frame_rect.h,
	};
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
