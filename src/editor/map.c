// TODO: fix bug where if the in_frame_rect is on another page it doesnt place it
// also random thought, in the API you should be able to pass a bitmask into the draw_map function
// so it only draws the tiles with those flags. That way you can do easy z sorting

#include "map.h"

#include <stdio.h>
#include <string.h>

#include "../backend/input.h"
#include "../backend/gui.h"
#include "../backend/gfx.h"
#include "shared.h"
#include "../res.h"

static camera_t _camera = {
	// Put camera at the center of the screen
	.pos = (point_t){
		.x = SCREEN_WIDTH / 2,
		.y = SCREEN_HEIGHT / 2,
	},
	.zoom = 1.0f,
	.screen_origin = (point_t){
		.x = SCREEN_WIDTH / 2,
		.y = SCREEN_HEIGHT / 2,
	},
};

// static const int _move_speed = 8;

// -1 is the entity layer
// 0 - 3 are tile layers
static int _selected_layer = 0;

#define ENTITY_LAYER -1

// TODO: also add this in the RAM but this variable should still exist
// because the editor shouldn't influence what layers are visible in-game
// but it should be able to be set in-game as well
static bool _hidden_layers[MAP_LAYERS_AMOUNT] = {0};

static bool _entity_layer_hidden = false;

static point_t _last_frame_mouse_pos = {0};

typedef enum entity_tool {
	ENTITY_TOOL_PICK,
	ENTITY_TOOL_SELECT,
	ENTITY_TOOL_MOVE,
	ENTITY_TOOL_STAMP,
} entity_tool_t;

ARRAY_DEFINE(size_t)

static size_t_array_t _selected_entity_indices = {0};

static entity_tool_t _selected_entity_tool = ENTITY_TOOL_SELECT;

static bool _moving_entity = false;
static point_t _moving_entity_offset = {0};
static point_t _entity_selection_start = {0};
static point_t _entity_selection_end = {0};

static struct {
	rect_t map_rect;

	point_t entity_layer_pos;
	point_t layer_buttons_start_pos;

	// Will remove after I've got skins implemented
	rect_t gui_rect;

	point_t sprite_selector_pos;
	point_t sprite_selector_buttons_start_pos;

	point_t tools_start_pos;
}
_layout = {
	.map_rect = {{0, 20, 620, 324}},
	.gui_rect = {{0, 344, 640, 136}},

	.entity_layer_pos = {4, 394},
	.layer_buttons_start_pos = {4, 412},
	
	.sprite_selector_pos = {200, 348},
	.sprite_selector_buttons_start_pos = {588, 348},

	.tools_start_pos = {622, 34},
};

static void _draw_grid(surface_t surf) {
	// TODO: make color part of skin
	// TODO: Don't hardcode the 16, I should make a macro for that as well
	// Maybe even use constexpr??? but then I need C23 I think
	
	// Horizontal
	for (int i = 0; i <= 16; i++) {
		point_t start = POINT(0, i * SCREEN_HEIGHT);
		point_t end = POINT(16 * SCREEN_WIDTH, i * SCREEN_HEIGHT);

		gfx_draw_line(surf, cam_world_to_screen(&_camera, start), cam_world_to_screen(&_camera, end), 7);
	}

	// Vertical
	for (int i = 0; i <= 16; i++) {
		point_t start = POINT(i * SCREEN_WIDTH, 0);
		point_t end = POINT(i * SCREEN_WIDTH, 16 * SCREEN_HEIGHT);

		gfx_draw_line(surf, cam_world_to_screen(&_camera, start), cam_world_to_screen(&_camera, end), 7);
	}
}

void map_editor_init(computer_t *computer) {
	// TODO: dealloc
	array_init(&_selected_entity_indices, get_heap_allocator());
}

void map_editor_update(computer_t *computer) {
	sprite_selector_update(computer, SNAP_MODE_ZOOM, _layout.sprite_selector_pos);

	point_t mouse_pos = input_get_mouse_pos();
	rect_t in_frame_rect = get_in_frame_rect();
	rect_t in_frame_rect_in_sprites = get_in_frame_rect_in_sprites();

	if (_selected_layer == ENTITY_LAYER && point_in_rect(mouse_pos, _layout.map_rect)) { // Entities layer
		if (_selected_entity_tool == ENTITY_TOOL_SELECT) {
			if (input_mouse_button_pressed(MOUSE_BUTTON_LEFT)) {
				_moving_entity = false;

				array_clear(&_selected_entity_indices);
				
				point_t world_mouse_pos = cam_screen_to_world(&_camera, mouse_pos);

				for (size_t i = 0; i < MAX_ENTITIES; i++) {
					if (computer->ram->entities.entities[i].id[0] == '\0') {
						break;
					}

					rect_t entity_rect = {
						.x = computer->ram->entities.entities[i].x,
						.y = computer->ram->entities.entities[i].y,
						.w = computer->ram->entities.entities[i].w * SPRITE_WIDTH,
						.h = computer->ram->entities.entities[i].h * SPRITE_HEIGHT,
					};

					if (point_in_rect(world_mouse_pos, entity_rect)) {
						array_push(&_selected_entity_indices, i);
						_moving_entity = true;

						_moving_entity_offset.x = world_mouse_pos.x - computer->ram->entities.entities[i].x;
						_moving_entity_offset.y = world_mouse_pos.y - computer->ram->entities.entities[i].y;

						break;
					}

					_entity_selection_start = world_mouse_pos;
				}
			}

			if (input_mouse_button_held(MOUSE_BUTTON_LEFT)) {
				point_t world_mouse_pos = cam_screen_to_world(&_camera, mouse_pos);

				if (_moving_entity) {
					input_set_cursor_style(CURSOR_STYLE_MOVE);
					size_t index = _selected_entity_indices.data[0];
					computer->ram->entities.entities[index].x = world_mouse_pos.x - _moving_entity_offset.x;
					computer->ram->entities.entities[index].y = world_mouse_pos.y - _moving_entity_offset.y;
				} else {
					_entity_selection_end = world_mouse_pos;
				}
			}

			if (input_mouse_button_released(MOUSE_BUTTON_LEFT) && !_moving_entity) {
				// Add all entities within selection rect to _selected_entity_indices
			}
		} else if (_selected_entity_tool == ENTITY_TOOL_STAMP) {
			if (input_mouse_button_pressed(MOUSE_BUTTON_LEFT)) {
				// Kind of inefficient, should improve if it becomes problematic
				for (size_t i = 0; i < MAX_ENTITIES; i++) {
					if (computer->ram->entities.entities[i].id[0] == '\0') {
						// Found empty entity
	
						// strncpy((char *)computer->ram->entities.entities[i].id, "idk", 3);
						computer->ram->entities.entities[i].id[0] = 'e';
	
						point_t pos = cam_screen_to_world(&_camera, POINT(mouse_pos.x - (in_frame_rect.w * _camera.zoom) / 2, mouse_pos.y - (in_frame_rect.h * _camera.zoom) / 2));
						computer->ram->entities.entities[i].x = pos.x;
						computer->ram->entities.entities[i].y = pos.y;
						
						computer->ram->entities.entities[i].sprite = get_sprite_index();
						computer->ram->entities.entities[i].w = in_frame_rect_in_sprites.w;
						computer->ram->entities.entities[i].h = in_frame_rect_in_sprites.h;
	
						break;
					}
				}
			}
		}
	} else { // Tile layers
		point_t cell_mouse_pos = cam_screen_to_tile(&_camera, mouse_pos, in_frame_rect_in_sprites, SPRITE_WIDTH, SPRITE_HEIGHT);

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

	// Zoom for sprite selector
	if (point_in_rect(mouse_pos, RECT(_layout.sprite_selector_pos.x, _layout.sprite_selector_pos.y, SPRITESHEET_PAGE_WIDTH, SPRITESHEET_PAGE_HEIGHT))) {
		if (input_key_pressed(KEY_MINUS) || input_mouse_scrolled(SCROLL_DIR_UP)) {
			sprite_selector_zoom_in();
		}

		if (input_key_pressed(KEY_EQUALS) || input_mouse_scrolled(SCROLL_DIR_DOWN)) {
			sprite_selector_zoom_out();
		}
	}

	// if (input_key_held(KEY_A)) {
	// 	_camera.pos.x -= _move_speed;
	// }
	// if (input_key_held(KEY_D)) {
	// 	_camera.pos.x += _move_speed;
	// }
	// if (input_key_held(KEY_W)) {
	// 	_camera.pos.y -= _move_speed;
	// }
	// if (input_key_held(KEY_S)) {
	// 	_camera.pos.y += _move_speed;
	// }

	point_t mouse_in_world = cam_screen_to_world(&_camera, mouse_pos);
	
	if (input_mouse_button_held(MOUSE_BUTTON_MIDDLE)) {
		point_t diff = {
			.x = mouse_pos.x - _last_frame_mouse_pos.x,
			.y = mouse_pos.y - _last_frame_mouse_pos.y,
		};

		_camera.pos.x -= diff.x / _camera.zoom;
		_camera.pos.y -= diff.y / _camera.zoom;

		input_set_cursor_style(CURSOR_STYLE_HAND);
	}

	if (point_in_rect(mouse_pos, _layout.map_rect)) {
		if (input_mouse_scrolled(SCROLL_DIR_UP) && _camera.zoom < 4.0f) {
			float new_zoom = _camera.zoom * 2.0f;
			
			_camera.pos.x += (mouse_in_world.x - _camera.pos.x) * (1 - _camera.zoom / new_zoom);
			_camera.pos.y += (mouse_in_world.y - _camera.pos.y) * (1 - _camera.zoom / new_zoom);
			
			_camera.zoom = new_zoom;
		}

		if (input_mouse_scrolled(SCROLL_DIR_DOWN) && _camera.zoom >= 0.25f) {
			float new_zoom = _camera.zoom * 0.5f;
			
			_camera.pos.x += (mouse_in_world.x - _camera.pos.x) * (1 - _camera.zoom / new_zoom);
			_camera.pos.y += (mouse_in_world.y - _camera.pos.y) * (1 - _camera.zoom / new_zoom);
			
			_camera.zoom = new_zoom;
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
		.w = (int)(SCREEN_WIDTH / (SPRITE_WIDTH * _camera.zoom)) + 1,
		.h = (int)(SCREEN_HEIGHT / (SPRITE_HEIGHT * _camera.zoom)) + 1,
	};

	section.pos = cam_screen_to_world(&_camera, (point_t){0});
	section.pos.x /= SPRITE_WIDTH;
	section.pos.y /= SPRITE_HEIGHT;

	{
		point_t top_left = cam_world_to_screen(&_camera, POINT(0, 0));
		point_t bottom_right = cam_world_to_screen(&_camera, POINT(MAP_WIDTH * SPRITE_WIDTH, MAP_HEIGHT * SPRITE_HEIGHT));
		
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
				cam_world_to_screen(&_camera, (point_t){0}),
				section,
				_camera.zoom,
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
			
			point_t pos = cam_world_to_screen(&_camera, POINT(computer->ram->entities.entities[i].x, computer->ram->entities.entities[i].y));

			rect_t dest_rect = {
				.x = pos.x,
				.y = pos.y,
				.w = source_rect.w * _camera.zoom,
				.h = source_rect.h * _camera.zoom,
			};

			// TODO: change color key to that of the sprite?
			// but which sprite
			gfx_draw_spritesheet_pro(computer->ram, source_rect, dest_rect, COLOR_BLACK, _layout.map_rect);

			// if (computer->ram->entities.entities[i].selected) {
			// 	// TODO: maybe make a function of expand_rect(rect_t rect, int amount);
			// 	draw_selection_rect(0, fb_surf, dest_rect);
			// }
		}
	}

	// Draw selection rect
	for (size_t i = 0; i < _selected_entity_indices.len; i++) {
		entity_t *entity = &computer->ram->entities.entities[_selected_entity_indices.data[i]];

		rect_t source_rect = sprite_index_to_spritesheet_rect(entity->sprite, entity->w, entity->h);
		point_t pos = cam_world_to_screen(&_camera, POINT(entity->x, entity->y));

		rect_t dest_rect = {
			.x = pos.x,
			.y = pos.y,
			.w = source_rect.w * _camera.zoom,
			.h = source_rect.h * _camera.zoom,
		};

		// TODO: maybe make a function of expand_rect(rect_t rect, int amount);
		gui_draw_selection_rect(computer->ram->ticks, fb_surf, dest_rect);
	}

	// Draw rect where mouse is
	point_t mouse_pos = input_get_mouse_pos();
	rect_t in_frame_rect = get_in_frame_rect();
	rect_t in_frame_rect_in_sprites = get_in_frame_rect_in_sprites();

	_draw_grid(fb_surf);

	// Draw skin again because currently I don't have a way to clip the gfx_draw_map function (yet)
	// TODO: make a better solution for this
	surface_t skin_surface = (surface_t){.data = computer->ram->skin.data, .width = SKIN_WIDTH, .height = SKIN_HEIGHT};
	gfx_draw_surface_rect(&computer->ram->framebuffer, skin_surface, POINT(0, 0), RECT(SCREEN_WIDTH * 2, 0, SCREEN_WIDTH, SCREEN_HEIGHT), computer->ram->skin.color_key);

	if (_selected_layer == ENTITY_LAYER) {
		if (_selected_entity_tool == ENTITY_TOOL_SELECT) {
			// TODO
		} else if (_selected_entity_tool == ENTITY_TOOL_STAMP) {
			rect_t dest_rect = {
				.x = mouse_pos.x - (in_frame_rect.w * _camera.zoom) / 2,
				.y = mouse_pos.y - (in_frame_rect.h * _camera.zoom) / 2,
				.w = in_frame_rect.w * _camera.zoom,
				.h = in_frame_rect.h * _camera.zoom,
			};
	
			dest_rect.pos = snap_to_grid(dest_rect.pos, (int)_camera.zoom, (int)_camera.zoom);
	
			gfx_draw_spritesheet_pro(computer->ram, in_frame_rect, dest_rect, COLOR_BLACK, _layout.map_rect); // TODO: replace COLOR_NONE with the color key of the sprite
		}
	} else {
		if (point_in_rect(mouse_pos, _layout.map_rect)) {
			point_t tile = cam_screen_to_tile(&_camera, mouse_pos, in_frame_rect_in_sprites, SPRITE_WIDTH, SPRITE_HEIGHT);
			point_t rect_pos = cam_tile_to_screen(&_camera, tile, in_frame_rect_in_sprites, SPRITE_WIDTH, SPRITE_HEIGHT);
			rect_pos.x -= 1;
			rect_pos.y -= 1;
			gfx_draw_rect(fb_surf, RECT(rect_pos.x, rect_pos.y, in_frame_rect.w * _camera.zoom + 2, in_frame_rect.h * _camera.zoom + 2), COLOR_WHITE);
		}
	}

	sprite_selector_draw(computer, _layout.sprite_selector_pos, _layout.sprite_selector_buttons_start_pos);

	// Entity layer
	if (gui_button(computer->ram, _layout.entity_layer_pos, g_skin_layout.map_entity_layer_button, _selected_layer == ENTITY_LAYER)) {
		_selected_layer = ENTITY_LAYER;
	}

	// Entity layer visible
	_entity_layer_hidden = gui_toggle_button(computer->ram, POINT(_layout.entity_layer_pos.x + g_skin_layout.map_entity_layer_button.pressed_rect.w, _layout.entity_layer_pos.y), g_skin_layout.toggle_layer_button, _entity_layer_hidden);

	// Other layers
	for (int i = 0; i < g_skin_layout.map_layer_buttons.amount; i++) {
		point_t pos = button_array_get_pos(&g_skin_layout.map_layer_buttons, _layout.layer_buttons_start_pos, i);
		button_t button = button_array_get(&g_skin_layout.map_layer_buttons, i);

		if (gui_button(computer->ram, pos, button, _selected_layer == i)) {
			_selected_layer = i;
		}

		// Visibility button
		_hidden_layers[i] = gui_toggle_button(computer->ram, POINT(pos.x + button.pressed_rect.w, pos.y), g_skin_layout.toggle_layer_button, _hidden_layers[i]);
	}

	// Tool bar
	if (_selected_layer == ENTITY_LAYER) {
		for (int i = 0; i < g_skin_layout.map_entity_tool_buttons.amount; i++) {
			point_t pos = button_array_get_pos(&g_skin_layout.map_entity_tool_buttons, _layout.tools_start_pos, i);
			button_t button = button_array_get(&g_skin_layout.map_entity_tool_buttons, i);

			if (gui_button(computer->ram, pos, button, i == _selected_entity_tool)) {
				_selected_entity_tool = i;
			}
		}
	} else {

	}
}
