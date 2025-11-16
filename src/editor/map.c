#include "map.h"

#include <stdio.h>
#include <string.h>

#include "menu.h"
#include "../backend/input.h"
#include "../backend/gui.h"
#include "../backend/gfx.h"
#include "shared.h"

// TODO: wrap into camera struct
// Put camera at the center of the screen
static point_t _cam_pos = {
	.x = SCREEN_WIDTH / 2,
	.y = SCREEN_HEIGHT / 2,
};
static float _zoom = 1.0f;

static const int _move_speed = 8;


// -1 is the entity layer
// 0 - 3 are tile layers
static int _selected_layer = 0;

// TODO: also add this in the RAM but this variable should still exist
// because the editor shouldn't influence what layers are visible in-game
// but it should be able to be set in-game as well
static bool _hidden_layers[MAP_LAYERS_AMOUNT] = {0};

static bool _entity_layer_hidden = false;

static point_t _last_frame_mouse_pos = {0};

typedef struct layout {
	rect_t map_rect;

	point_t entity_layer_pos;
	point_t layer_buttons_start_pos;

	// Will remove after I've got skins implemented
	rect_t gui_rect;

	point_t sprite_selector_pos;
	point_t sprite_selector_buttons_start_pos;
} layout_t;

static const layout_t _layout = {
	.map_rect = {{0, 20, 640, 324}},
	.gui_rect = {{0, 344, 640, 136}},

	.entity_layer_pos = {4, 394},
	.layer_buttons_start_pos = {4, 412},
	
	.sprite_selector_pos = {200, 348},
	.sprite_selector_buttons_start_pos = {588, 348},
};

// TODO: use
typedef struct camera {
	point_t pos;
	float scale;
} camera_t;

static point_t _point_world_to_screen(point_t world) {
	return (point_t){
		.x = (int)((world.x - _cam_pos.x + (SCREEN_WIDTH / (2.0f * _zoom))) * _zoom),
		.y = (int)((world.y - _cam_pos.y + (SCREEN_HEIGHT / (2.0f * _zoom))) * _zoom),
	};
}

// Convert screen coordinates to world coordinates
static point_t _point_screen_to_world(point_t screen) {
	return (point_t){
		.x = (int)((screen.x / _zoom) + _cam_pos.x - (SCREEN_WIDTH / (2.0f * _zoom))),
		.y = (int)((screen.y / _zoom) + _cam_pos.y - (SCREEN_HEIGHT / (2.0f * _zoom))),
	};
}

// Snap a world point to the grid (cell size in pixels)
static point_t _point_snap_to_grid(point_t world, int cell_w, int cell_h) {
	if (cell_w == 0) cell_w = 1;
	if (cell_h == 0) cell_h = 1;

	return (point_t){
		.x = (world.x / cell_w) * cell_w,
		.y = (world.y / cell_h) * cell_h,
	};
}

// Converts screen coordinates to tile indices in a layer (rect_in_tiles is in tiles)
static point_t _point_screen_to_tile(point_t screen, rect_t rect_in_tiles, int cell_w, int cell_h) {
	point_t world = _point_screen_to_world(screen);
	return (point_t){
		.x = (world.x / (rect_in_tiles.w * cell_w)) * rect_in_tiles.w,
		.y = (world.y / (rect_in_tiles.h * cell_h)) * rect_in_tiles.h,
	};
}

// Converts tile coordinates back to screen coordinates (rect_in_tiles is in tiles)
static point_t _point_tile_to_screen(point_t tile, rect_t rect_in_tiles, int cell_w, int cell_h) {
	point_t world = (point_t){
		.x = ((tile.x * rect_in_tiles.w * cell_w)) / rect_in_tiles.w,
		.y = ((tile.y * rect_in_tiles.h * cell_h)) / rect_in_tiles.h,
	};
	return _point_world_to_screen(world);
}

static void _draw_grid(surface_t surf) {
	// TODO: make color part of skin
	// TODO: Don't hardcode the 16, I should make a macro for that as well
	// Maybe even use constexpr??? but then I need C23 I think
	
	// Horizontal
	for (int i = 0; i <= 16; i++) {
		point_t start = POINT(0, i * SCREEN_HEIGHT);
		point_t end = POINT(16 * SCREEN_WIDTH, i * SCREEN_HEIGHT);

		gfx_draw_line(surf, _point_world_to_screen(start), _point_world_to_screen(end), 7);
	}

	// Vertical
	for (int i = 0; i <= 16; i++) {
		point_t start = POINT(i * SCREEN_WIDTH, 0);
		point_t end = POINT(i * SCREEN_WIDTH, 16 * SCREEN_HEIGHT);

		gfx_draw_line(surf, _point_world_to_screen(start), _point_world_to_screen(end), 7);
	}
}

void map_editor_init(computer_t *computer) {

}

void map_editor_update(computer_t *computer) {
	sprite_selector_update(computer, SNAP_MODE_ZOOM, _layout.sprite_selector_pos);

	point_t mouse_pos = input_get_mouse_pos();
	rect_t in_frame_rect = get_in_frame_rect();
	rect_t in_frame_rect_in_sprites = get_in_frame_rect_in_sprites();

	if (_selected_layer == -1) { // Entities layer
		if (point_in_rect(mouse_pos, _layout.map_rect)) {
			if (input_mouse_button_pressed(MOUSE_BUTTON_LEFT)) {
				// Kind of inefficient, should improve if it becomes problematic
				for (size_t i = 0; i < MAX_ENTITIES; i++) {
					if (computer->ram->entities.entities[i].id[0] == '\0') {
						// Found empty entity

						// strncpy((char *)computer->ram->entities.entities[i].id, "idk", 3);
						computer->ram->entities.entities[i].id[0] = 'e';

						point_t pos = _point_screen_to_world(POINT(mouse_pos.x - (in_frame_rect.w * _zoom) / 2, mouse_pos.y - (in_frame_rect.h * _zoom) / 2));
						computer->ram->entities.entities[i].x = pos.x;
						computer->ram->entities.entities[i].y = pos.y;
						
						computer->ram->entities.entities[i].sprite = get_absolute_sprite_index();
						computer->ram->entities.entities[i].w = in_frame_rect_in_sprites.w;
						computer->ram->entities.entities[i].h = in_frame_rect_in_sprites.h;

						break;
					}
				}
			}
		}
	} else { // Tile layers
		point_t cell_mouse_pos = _point_screen_to_tile(mouse_pos, in_frame_rect_in_sprites, SPRITE_WIDTH, SPRITE_HEIGHT);

		if (point_in_rect(mouse_pos, _layout.map_rect)) {
			if (input_mouse_button_held(MOUSE_BUTTON_LEFT)) {
				for (int i = 0; i < in_frame_rect_in_sprites.h; i++) {
					for (int j = 0; j < in_frame_rect_in_sprites.w; j++) {
						// TODO: make a function for this
						int index = (get_page_index() * SPRITES_PER_PAGE) + sprite_coords_to_index(in_frame_rect_in_sprites.x + j, in_frame_rect_in_sprites.y + i);
						computer->ram->map.layers[_selected_layer].data[(cell_mouse_pos.y + i) * MAP_WIDTH + (cell_mouse_pos.x + j)] = (uint16_t)index;
					}
				}
			}
		
			if (input_mouse_button_held(MOUSE_BUTTON_RIGHT)) {
				for (int i = 0; i < in_frame_rect_in_sprites.h; i++) {
					for (int j = 0; j < in_frame_rect_in_sprites.w; j++) {
						computer->ram->map.layers[_selected_layer].data[(cell_mouse_pos.y + i) * MAP_WIDTH + (cell_mouse_pos.x + j)] = 0;
					}
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

	point_t mouse_in_world = _point_screen_to_world(mouse_pos);
	
	if (input_mouse_button_held(MOUSE_BUTTON_MIDDLE)) {
		point_t diff = {
			.x = mouse_pos.x - _last_frame_mouse_pos.x,
			.y = mouse_pos.y - _last_frame_mouse_pos.y,
		};

		_cam_pos.x -= diff.x / _zoom;
		_cam_pos.y -= diff.y / _zoom;
	}

	if (point_in_rect(mouse_pos, _layout.map_rect)) {
		if (input_mouse_scrolled(SCROLL_DIR_UP) && _zoom < 4.0f) {
			float new_zoom = _zoom * 2.0f;
			
			_cam_pos.x += (mouse_in_world.x - _cam_pos.x) * (1 - _zoom / new_zoom);
			_cam_pos.y += (mouse_in_world.y - _cam_pos.y) * (1 - _zoom / new_zoom);
			
			_zoom = new_zoom;
		}

		if (input_mouse_scrolled(SCROLL_DIR_DOWN) && _zoom >= 0.25f) {
			float new_zoom = _zoom * 0.5f;
			
			_cam_pos.x += (mouse_in_world.x - _cam_pos.x) * (1 - _zoom / new_zoom);
			_cam_pos.y += (mouse_in_world.y - _cam_pos.y) * (1 - _zoom / new_zoom);
			
			_zoom = new_zoom;
		}
	}

	_last_frame_mouse_pos = mouse_pos;
}

static void _draw_background(framebuffer_t *fb, rect_t rect, color_t color1, color_t color2, int width) {
	int offset = 0;
	for (int i = 0; i < rect.h; i++) {
		for (int j = 0; j < rect.w; j++) {
			
			if (j % width < width / 2) {
				gfx_set_pixel(fb, (rect.x + j + offset) % rect.w, rect.y + i, color1);
			} else {
				gfx_set_pixel(fb, (rect.x + j + offset) % rect.w, rect.y + i, color2);
			}
		}
		// printf("offset: %d\n", (offset % rect.w));
		offset++;
	}
}

void map_editor_draw(computer_t *computer) {
	surface_t fb_surf = FB_SURF(computer->ram->framebuffer.data);

	// TODO: make colors part of skin
	_draw_background(&computer->ram->framebuffer, _layout.map_rect, 0, 4, 8);
	// gfx_draw_filled_rect(fb_surf, _layout.map_rect, 0);

	// Section of the map that's visible
	// So the map drawing is O(1)
	rect_t section = {
		.w = (int)(SCREEN_WIDTH / (SPRITE_WIDTH * _zoom)) + 1,
		.h = (int)(SCREEN_HEIGHT / (SPRITE_HEIGHT * _zoom)) + 1,
	};

	section.pos = _point_screen_to_world((point_t){0});
	section.pos.x /= SPRITE_WIDTH;
	section.pos.y /= SPRITE_HEIGHT;

	{
		point_t top_left = _point_world_to_screen(POINT(0, 0));
		point_t bottom_right = _point_world_to_screen(POINT(MAP_WIDTH * SPRITE_WIDTH, MAP_HEIGHT * SPRITE_HEIGHT));
		
		rect_t section_in_pixels = {0};
		section_in_pixels.pos = top_left;
		section_in_pixels.size.x = bottom_right.x - top_left.x;
		section_in_pixels.size.y = bottom_right.y - top_left.y;
		
		// Clip for optimization
		section_in_pixels = rect_clip(_layout.map_rect, section_in_pixels);

		gfx_draw_filled_rect(fb_surf, section_in_pixels, 0);
	}

	for (int i = 0; i < MAP_LAYERS_AMOUNT; i++) {
		if (!_hidden_layers[i]) {
			gfx_draw_map(
				computer->ram,
				i,
				_point_world_to_screen((point_t){0}),
				section,
				_zoom,
				COLOR_BLACK
			);
		}
	}

	// Draw entities
	if (!_entity_layer_hidden) {
		for (size_t i = 0; i < MAX_ENTITIES; i++) {
			if (computer->ram->entities.entities[i].id[0] == '\0') {
				break;
			}
	
			rect_t source_rect = sprite_index_to_spritesheet_rect(computer->ram->entities.entities[i].sprite, computer->ram->entities.entities[i].w, computer->ram->entities.entities[i].h);
			
			point_t pos = _point_world_to_screen(POINT(computer->ram->entities.entities[i].x, computer->ram->entities.entities[i].y));

			rect_t dest_rect = {
				.x = pos.x,
				.y = pos.y,
				.w = source_rect.w * _zoom,
				.h = source_rect.h * _zoom,
			};

			// TODO: change color key to that of the sprite?
			// but which sprite
			gfx_draw_spritesheet_pro(computer->ram, source_rect, dest_rect, COLOR_BLACK);
		}
	}

	// Draw rect where mouse is
	point_t mouse_pos = input_get_mouse_pos();
	rect_t in_frame_rect = get_in_frame_rect();
	rect_t in_frame_rect_in_sprites = get_in_frame_rect_in_sprites();

	_draw_grid(fb_surf);

	// Draw skin again because currently I don't have a way to clip the gfx_draw_map function
	surface_t skin_surface = (surface_t){.data = computer->ram->skin.data, .width = SKIN_WIDTH, .height = SKIN_HEIGHT};
	gfx_draw_surface_rect(&computer->ram->framebuffer, skin_surface, POINT(0, 0), RECT(SCREEN_WIDTH * 2, 0, SCREEN_WIDTH, SCREEN_HEIGHT), computer->ram->skin.color_key);

	if (_selected_layer == -1) {
		rect_t dest_rect = {
			.x = mouse_pos.x - (in_frame_rect.w * _zoom) / 2,
			.y = mouse_pos.y - (in_frame_rect.h * _zoom) / 2,
			.w = in_frame_rect.w * _zoom,
			.h = in_frame_rect.h * _zoom,
		};

		dest_rect.pos = _point_snap_to_grid(dest_rect.pos, (int)_zoom, (int)_zoom);

		gfx_draw_spritesheet_pro(computer->ram, in_frame_rect, dest_rect, COLOR_BLACK); // TODO: replace COLOR_NONE with the color key of the sprite

	} else {
		if (point_in_rect(mouse_pos, _layout.map_rect)) {
			point_t tile = _point_screen_to_tile(mouse_pos, in_frame_rect_in_sprites, SPRITE_WIDTH, SPRITE_HEIGHT);
			point_t rect_pos = _point_tile_to_screen(tile, in_frame_rect_in_sprites, SPRITE_WIDTH, SPRITE_HEIGHT);
			rect_pos.x -= 1;
			rect_pos.y -= 1;
			gfx_draw_rect(fb_surf, RECT(rect_pos.x, rect_pos.y, in_frame_rect.w * _zoom + 2, in_frame_rect.h * _zoom + 2), COLOR_WHITE);
		}
	}

	sprite_selector_draw(computer, _layout.sprite_selector_pos, _layout.sprite_selector_buttons_start_pos);

	// Entity layer
	if (gui_button(computer->ram, _layout.entity_layer_pos, skin_layout.map_entity_layer_button, _selected_layer == -1)) {
		_selected_layer = -1;
	}

	// Entity layer visible
	_entity_layer_hidden = gui_toggle_button(computer->ram, POINT(_layout.entity_layer_pos.x + skin_layout.map_entity_layer_button.pressed_rect.w, _layout.entity_layer_pos.y), skin_layout.toggle_layer_button, _entity_layer_hidden);

	// Other layers
	for (int i = 0; i < skin_layout.map_layer_buttons.amount; i++) {
		point_t pos = button_array_get_pos(&skin_layout.map_layer_buttons, _layout.layer_buttons_start_pos, i);
		button_t button = button_array_get(&skin_layout.map_layer_buttons, i);

		if (gui_button(computer->ram, pos, button, _selected_layer == i)) {
			_selected_layer = i;
		}

		// Visibility button
		_hidden_layers[i] = gui_toggle_button(computer->ram, POINT(pos.x + button.pressed_rect.w, pos.y), skin_layout.toggle_layer_button, _hidden_layers[i]);
	}
}
