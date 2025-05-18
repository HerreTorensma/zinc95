#include "map.h"

#include <stdio.h>

#include "menu.h"
#include "../backend/input.h"
#include "../backend/gui.h"
#include "../backend/gfx.h"
#include "shared.h"

static point_t _cam_pos = {0};
static const int _move_speed = 8;

// Doesn't do anything yet
static float _zoom = 1.0f;
static int _selected_layer = 0;

typedef struct layout {
	rect_t map_rect;
	point_t layer_buttons_start_pos;

	// Will remove after I've got skins implemented
	rect_t gui_rect;

	rect_t spritesheet_rect;
	point_t spritesheet_pages_start_pos;
} layout_t;

static const layout_t _layout = {
	.map_rect = {{0, 20, 640, 324}},
	.gui_rect = {{0, 344, 640, 136}},

	.layer_buttons_start_pos = {2, 346},
	
	.spritesheet_rect = {{200, 348, 384, 128}},
	.spritesheet_pages_start_pos = {588, 348},
};

// TODO: i need some kind of function to translate world coords to screen coords and grid coords or whatever
// Instead or hardcoding it

static void _draw_grid(framebuffer_t *fb) {
	int line_x = SCREEN_WIDTH - _cam_pos.x % SCREEN_WIDTH;
	int line_y = SCREEN_HEIGHT - _cam_pos.y % SCREEN_HEIGHT;

	if (line_x >= SCREEN_WIDTH) {
		line_x = -_cam_pos.x;
	}
	if (line_y >= SCREEN_HEIGHT) {
		line_y = -_cam_pos.y;
	}

	gfx_draw_line(fb, POINT(0, line_y), POINT(SCREEN_WIDTH, line_y), 7);
	gfx_draw_line(fb, POINT(line_x, 0), POINT(line_x, SCREEN_HEIGHT), 7);
}

void map_editor_init(computer_t *computer) {

}

void map_editor_update(computer_t *computer) {
	sprite_selector_update(computer, SNAP_MODE_ZOOM, _layout.spritesheet_rect);

	point_t mouse_pos = input_get_mouse_pos();

	int cell_x = ((mouse_pos.x + _cam_pos.x) / currently_editing_rect.w) * currently_editing_sprites_rect.w;
	int cell_y = ((mouse_pos.y + _cam_pos.y) / currently_editing_rect.h) * currently_editing_sprites_rect.h;

	if (point_in_rect(mouse_pos, _layout.map_rect)) {
		if (input_mouse_button_held(MOUSE_BUTTON_LEFT)) {
			for (int i = 0; i < currently_editing_sprites_rect.h; i++) {
				for (int j = 0; j < currently_editing_sprites_rect.w; j++) {
					// TODO: make a function for this
					int index = (selected_spritesheet_index * SPRITES_PER_PAGE) + sprite_coords_to_index(currently_editing_sprites_rect.x + j, currently_editing_sprites_rect.y + i);
					computer->ram->map.layers[_selected_layer].data[(cell_y + i) * MAP_WIDTH + (cell_x + j)] = (uint16_t)index;
				}
			}
		}
	
		if (input_mouse_button_held(MOUSE_BUTTON_RIGHT)) {
			for (int i = 0; i < currently_editing_sprites_rect.h; i++) {
				for (int j = 0; j < currently_editing_sprites_rect.w; j++) {
					computer->ram->map.layers[_selected_layer].data[(cell_y + i) * MAP_WIDTH + (cell_x + j)] = 0;
				}
			}
		}
	}

	if (input_key_held(KEY_A)) {
		_cam_pos.x -= _move_speed;
	}
	if (input_key_held(KEY_D)) {
		_cam_pos.x += _move_speed;
	}
	if (input_key_held(KEY_W)) {
		_cam_pos.y -= _move_speed;
	}
	if (input_key_held(KEY_S)) {
		_cam_pos.y += _move_speed;
	}

	if (input_mouse_scrolled(SCROLL_DIR_DOWN)) {
		_zoom *= 2.0f;
	}
	if (input_mouse_scrolled(SCROLL_DIR_UP)) {
		_zoom *= 0.5f;
	}
}

void map_editor_draw(computer_t *computer) {
	framebuffer_t *fb = &computer->ram->framebuffer;

	gfx_draw_filled_rect(fb, _layout.map_rect, 0);

	// Draw only the visible portion so we're not drawing the entire map
	// yeah
	int map_x = _cam_pos.x / SPRITE_WIDTH;
	int map_y = _cam_pos.y / SPRITE_HEIGHT;
	// 81 so the screen is always filled

	for (int i = 0; i < MAP_LAYERS_AMOUNT; i++) {
		gfx_draw_map(computer->ram, i, POINT(-_cam_pos.x, -_cam_pos.y), RECT(map_x, map_y, 81, 60));
	}

	// Draw rect where mouse is
	point_t mouse_pos = input_get_mouse_pos();

	if (point_in_rect(mouse_pos, _layout.map_rect)) {
		point_t rect_pos = {
			.x = ((mouse_pos.x + _cam_pos.x) / currently_editing_rect.w) * currently_editing_rect.w - _cam_pos.x,
			.y =  ((mouse_pos.y + _cam_pos.y) / currently_editing_rect.h) * currently_editing_rect.h - _cam_pos.y,
		};

		gfx_draw_rect(fb, RECT(rect_pos.x - 1, rect_pos.y - 1, currently_editing_rect.w + 2, currently_editing_rect.h + 2), COLOR_WHITE);
	}

	_draw_grid(fb);

	gui_outset_frame(computer->ram, _layout.gui_rect);
	sprite_selector_draw(computer, _layout.spritesheet_rect, _layout.spritesheet_pages_start_pos);

	gui_button(computer->ram, "Entities", RECT(_layout.layer_buttons_start_pos.x, _layout.layer_buttons_start_pos.y, 48, 16));

	// Layer buttons
	for (int i = 0; i < MAP_LAYERS_AMOUNT; i++) {
		char buffer[2];
		sprintf(buffer, "%d", i);

		rect_t rect = {
			.x = _layout.layer_buttons_start_pos.x,
			.y = _layout.layer_buttons_start_pos.y + (i + 1) * 16,
			.w = 48,
			.h = 16
		};

		if (gui_button_ex(computer->ram, buffer, rect, _selected_layer == i)) {
			_selected_layer = i;
		}
	}
}