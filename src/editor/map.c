#include "map.h"

#include <stdio.h>
#include <string.h>

#include "menu.h"
#include "../backend/input.h"
#include "../backend/gui.h"
#include "../backend/gfx.h"
#include "shared.h"

static point_t _cam_pos = {0};
static const int _move_speed = 8;

// Doesn't do anything yet
static float _zoom = 1.0f;

// -1 is the entity layer
// 0 - 3 are tile layers
static int _selected_layer = 0;

// TODO: also add this in the RAM but this variable should still exist
// because the editor shouldn't influence what layers are visible in-game
// but it should be able to be set in-game as well
static bool _hidden_layers[MAP_LAYERS_AMOUNT] = {0};

static bool _entity_layer_hidden = false;

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

// TODO: i need some kind of function to translate world coords to screen coords and grid coords or whatever
// Instead or hardcoding it

static void _draw_grid(surface_t surf) {
	int line_x = SCREEN_WIDTH - _cam_pos.x % SCREEN_WIDTH;
	int line_y = SCREEN_HEIGHT - _cam_pos.y % SCREEN_HEIGHT;

	if (line_x >= SCREEN_WIDTH) {
		line_x = -_cam_pos.x;
	}
	if (line_y >= SCREEN_HEIGHT) {
		line_y = -_cam_pos.y;
	}

	gfx_draw_line(surf, POINT(0, line_y), POINT(SCREEN_WIDTH, line_y), 7);
	gfx_draw_line(surf, POINT(line_x, 0), POINT(line_x, SCREEN_HEIGHT), 7);
}

// static void _spawn_entity(entities)

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

						point_t pos = {
							.x = mouse_pos.x - in_frame_rect.w / 2,
							.y = mouse_pos.y - in_frame_rect.h / 2,
						};
						computer->ram->entities.entities[i].x = pos.x + _cam_pos.x;
						computer->ram->entities.entities[i].y = pos.y + _cam_pos.y;
						
						computer->ram->entities.entities[i].sprite = get_absolute_sprite_index();
						computer->ram->entities.entities[i].w = in_frame_rect_in_sprites.w;
						computer->ram->entities.entities[i].h = in_frame_rect_in_sprites.h;

						break;
					}
				}
			}
		}
	} else { // Tile layers
		int cell_x = ((mouse_pos.x + _cam_pos.x) / in_frame_rect.w) * in_frame_rect_in_sprites.w;
		int cell_y = ((mouse_pos.y + _cam_pos.y) / in_frame_rect.h) * in_frame_rect_in_sprites.h;
		
		if (point_in_rect(mouse_pos, _layout.map_rect)) {
			if (input_mouse_button_held(MOUSE_BUTTON_LEFT)) {
				for (int i = 0; i < in_frame_rect_in_sprites.h; i++) {
					for (int j = 0; j < in_frame_rect_in_sprites.w; j++) {
						// TODO: make a function for this
						int index = (get_page_index() * SPRITES_PER_PAGE) + sprite_coords_to_index(in_frame_rect_in_sprites.x + j, in_frame_rect_in_sprites.y + i);
						computer->ram->map.layers[_selected_layer].data[(cell_y + i) * MAP_WIDTH + (cell_x + j)] = (uint16_t)index;
					}
				}
			}
		
			if (input_mouse_button_held(MOUSE_BUTTON_RIGHT)) {
				for (int i = 0; i < in_frame_rect_in_sprites.h; i++) {
					for (int j = 0; j < in_frame_rect_in_sprites.w; j++) {
						computer->ram->map.layers[_selected_layer].data[(cell_y + i) * MAP_WIDTH + (cell_x + j)] = 0;
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

	if (input_mouse_scrolled(SCROLL_DIR_DOWN)) {
		_zoom *= 2.0f;
	}
	if (input_mouse_scrolled(SCROLL_DIR_UP)) {
		_zoom *= 0.5f;
	}
}

void map_editor_draw(computer_t *computer) {
	surface_t fb_surf = FB_SURF(computer->ram->framebuffer.data);

	gfx_draw_filled_rect(fb_surf, _layout.map_rect, 0);

	// Draw only the visible portion so we're not drawing the entire map
	// yeah
	int map_x = _cam_pos.x / SPRITE_WIDTH;
	int map_y = _cam_pos.y / SPRITE_HEIGHT;
	// 81 so the screen is always filled

	for (int i = 0; i < MAP_LAYERS_AMOUNT; i++) {
		if (!_hidden_layers[i]) {
			gfx_draw_map(computer->ram, i, POINT(-_cam_pos.x, -_cam_pos.y), RECT(map_x, map_y, 81, 60));
		}
	}

	// Draw entities
	if (!_entity_layer_hidden) {
		for (size_t i = 0; i < MAX_ENTITIES; i++) {
			if (computer->ram->entities.entities[i].id[0] == '\0') {
				break;
			}
	
			point_t pos = POINT(computer->ram->entities.entities[i].x - _cam_pos.x, computer->ram->entities.entities[i].y - _cam_pos.y);
			rect_t rect = sprite_index_to_spritesheet_rect(computer->ram->entities.entities[i].sprite, computer->ram->entities.entities[i].w, computer->ram->entities.entities[i].h);
			gfx_draw_spritesheet_rect(computer->ram, pos, rect, COLOR_BLACK);
		}
	}

	_draw_grid(fb_surf);

	// Draw skin again because currently I don't have a way to clip the gfx_draw_map function
	surface_t skin_surface = (surface_t){.data = computer->ram->skin.data, .width = SKIN_WIDTH, .height = SKIN_HEIGHT};
	gfx_draw_surface_rect(&computer->ram->framebuffer, skin_surface, POINT(0, 0), RECT(SCREEN_WIDTH * 2, 0, SCREEN_WIDTH, SCREEN_HEIGHT), computer->ram->skin.color_key);

	// Draw rect where mouse is
	point_t mouse_pos = input_get_mouse_pos();
	rect_t in_frame_rect = get_in_frame_rect();
	rect_t in_frame_rect_in_sprites = get_in_frame_rect_in_sprites();

	if (_selected_layer == -1) {
		point_t pos = {
			.x = mouse_pos.x - in_frame_rect.w / 2,
			.y = mouse_pos.y - in_frame_rect.h / 2,
		};
		gfx_draw_spritesheet_rect(computer->ram, pos, in_frame_rect, COLOR_BLACK); // TODO: replace COLOR_NONE with the color key of the sprite
	} else {
		if (point_in_rect(mouse_pos, _layout.map_rect)) {
			point_t rect_pos = {
				.x = ((mouse_pos.x + _cam_pos.x) / in_frame_rect.w) * in_frame_rect.w - _cam_pos.x,
				.y =  ((mouse_pos.y + _cam_pos.y) / in_frame_rect.h) * in_frame_rect.h - _cam_pos.y,
			};
	
			gfx_draw_rect(fb_surf, RECT(rect_pos.x - 1, rect_pos.y - 1, in_frame_rect.w + 2, in_frame_rect.h + 2), COLOR_WHITE);
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
