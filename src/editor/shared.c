#include "shared.h"

#include "../backend/input.h"
#include "../backend/gfx.h"
#include "../backend/gui.h"
#include "../res.h"

static rect_t _visible_rect = {0};
static rect_t _in_frame_rect = {0}; // 'focused', in the white rect

static int _current_area_index = 0;

static void _set_area_index(int index) {
	_current_area_index = index;
	_visible_rect = (rect_t){
		.x = (_current_area_index % 4) * SPRITESHEET_PAGE_WIDTH,
		.y = (_current_area_index / 4) * SPRITESHEET_PAGE_HEIGHT,
		.w = SPRITESHEET_PAGE_WIDTH,
		.h = SPRITESHEET_PAGE_HEIGHT,
	};
}

void sprite_selector_init(computer_t *computer) {
	_set_area_index(0);
	
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
			ram->sprites[get_sprite_index()].color_key = 0;
			ram->sprites[get_sprite_index()].flags = 0U;
		}
	}
}

void sprite_selector_zoom_in() {
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

void sprite_selector_zoom_out() {
	/*
	_in_frame_rect.w += SPRITE_WIDTH;
	_in_frame_rect.h += SPRITE_HEIGHT;
	*/

	_in_frame_rect.w *= 2;
	_in_frame_rect.h *= 2;

	if (_in_frame_rect.h > SPRITESHEET_PAGE_HEIGHT) {
		_in_frame_rect.w /= 2;
		_in_frame_rect.h /= 2;
	}
}

// This whole function is kind of a mess and I should probably rewrite it at some point
void sprite_selector_update(computer_t *computer, sprite_select_snap_mode_t snap_mode, point_t pos) {
	point_t mouse_pos = input_get_mouse_pos();

	{
		point_t old_pos = _in_frame_rect.pos;

		if (is_keybind_pressed(g_keybinds.global.active_sprite_move_left)) {
			_in_frame_rect.x -= _in_frame_rect.w;
		}
		if (is_keybind_pressed(g_keybinds.global.active_sprite_move_right)) {
			_in_frame_rect.x += _in_frame_rect.w;
		}
		if (is_keybind_pressed(g_keybinds.global.active_sprite_move_up)) {
			_in_frame_rect.y -= _in_frame_rect.h;
		}
		if (is_keybind_pressed(g_keybinds.global.active_sprite_move_down)) {
			_in_frame_rect.y += _in_frame_rect.h;
		}

		if (_in_frame_rect.x < 0 || _in_frame_rect.y < 0 || _in_frame_rect.x >= SPRITESHEET_WIDTH || _in_frame_rect.y >= SPRITESHEET_HEIGHT) {
			_in_frame_rect.pos = old_pos;
		}

		if (_in_frame_rect.pos.x != old_pos.x || _in_frame_rect.pos.y != old_pos.y) {
			_set_area_index((_in_frame_rect.y / SPRITESHEET_PAGE_HEIGHT) * 4 + (_in_frame_rect.x / SPRITESHEET_PAGE_WIDTH));
		}
	}

	if (point_in_rect(mouse_pos, RECT(pos.x, pos.y, SPRITESHEET_PAGE_WIDTH, SPRITESHEET_PAGE_HEIGHT))) {
		if (input_mouse_button_held(MOUSE_BUTTON_LEFT)) {
			if (snap_mode == SNAP_MODE_SPRITE) {
				int adjusted_position_x = mouse_pos.x - pos.x;
				int adjusted_position_y = mouse_pos.y - pos.y;

				_in_frame_rect.x = (adjusted_position_x / SPRITE_WIDTH) * SPRITE_WIDTH;
				_in_frame_rect.y = (adjusted_position_y / SPRITE_HEIGHT) * SPRITE_HEIGHT;
				// _in_frame_rect.x = (adjusted_position_x / _in_frame_rect.w) * _in_frame_rect.w;
				// _in_frame_rect.y = (adjusted_position_y / _in_frame_rect.h) * _in_frame_rect.h;

				_in_frame_rect.x -= _in_frame_rect.w / 2;
				_in_frame_rect.y -= _in_frame_rect.h / 2;
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

				_in_frame_rect.x += _visible_rect.x;
				_in_frame_rect.y += _visible_rect.y;
			}
		}

		// TODO: implement
		// first I need a spritesheet_section_to_text kind of function
		// and its inverse
		// Also make undo work for this
		// And if you cut a sprite the map should also change
		// so you can easily reorder your spritesheet without messing up the whole map
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
	gfx_draw_spritesheet_rect(computer->ram, pos, _visible_rect, COLOR_NONE);

	int new_area_index = gui_full_button_matrix(computer->ram, page_buttons_pos, g_skin_layout.global.spritesheet_areas_button_matrix, _current_area_index);
	if (new_area_index != _current_area_index) {
		_set_area_index(new_area_index);
	}

	// gfx_draw_rect(FB_SURF(computer->ram->framebuffer.data), RECT(pos.x + _in_frame_rect.x - 1, pos.y + (_in_frame_rect.y % SPRITESHEET_PAGE_HEIGHT) - 1, _in_frame_rect.w + 2, _in_frame_rect.h + 2), 15);
	if (rect_in_rect(_visible_rect, _in_frame_rect)) {
		gfx_draw_rect(FB_SURF(computer->ram->framebuffer.data), RECT(pos.x + (_in_frame_rect.x % SPRITESHEET_PAGE_WIDTH) - 1, pos.y + (_in_frame_rect.y % SPRITESHEET_PAGE_HEIGHT) - 1, _in_frame_rect.w + 2, _in_frame_rect.h + 2), 15);
	}
}

rect_t get_page_rect() {
	return _visible_rect;
}

rect_t get_in_frame_rect() {
	return _in_frame_rect;
}

rect_t get_in_frame_rect_in_sprites() {
	return (rect_t) {
		.x = _in_frame_rect.x / SPRITE_WIDTH,
		.y = _in_frame_rect.y / SPRITE_HEIGHT,
		.w = _in_frame_rect.w / SPRITE_WIDTH,
		.h = _in_frame_rect.h / SPRITE_HEIGHT,
	};
}

int get_sprite_index() {
	rect_t rect = get_in_frame_rect_in_sprites();
	return rect.y * (SPRITESHEET_WIDTH / SPRITE_WIDTH) + rect.x;
}

int sprite_coords_to_index(int x, int y) {
	return y * (SPRITESHEET_WIDTH / SPRITE_WIDTH) + x;
}
