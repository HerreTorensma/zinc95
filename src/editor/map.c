#include "map.h"

#include <stdio.h>

#include "menu.h"
#include "../api/api.h"
#include "../backend/input.h"
#include "../backend/gui.h"
#include "shared.h"

static recti_t map_rect = {0};

static int cam_x = 0;
static int cam_y = 0;
static int move_speed = 8;

// enum {
// 	ZOOM_QUARTER,
// 	ZOOM_HALF,

// }

// Doesn't do anything yet
static float zoom = 1.0f;

static int selected_layer = 0;

// TODO: i need some kind of function to translate world coords to screen coords and grid coords or whatever
// Instead or hardcoding it

static void draw_grid(computer_t *computer) {
	int line_x = SCREEN_WIDTH - cam_x % SCREEN_WIDTH;
	int line_y = SCREEN_HEIGHT - cam_y % SCREEN_HEIGHT;

	if (line_x >= SCREEN_WIDTH) {
		line_x = -cam_x;
	}
	if (line_y >= SCREEN_HEIGHT) {
		line_y = -cam_y;
	}

	api_line(computer, 0, line_y, SCREEN_WIDTH, line_y, 7);
	api_line(computer, line_x, 0, line_x, SCREEN_HEIGHT, 7);
}

void map_editor_init(computer_t *computer) {
	map_rect = workspace_rect;
	map_rect.h -= 136;
}

void map_editor_update(computer_t *computer) {
	sprite_selector_update(computer, SNAP_MODE_ZOOM);

	vec2i_t mouse_pos = input_get_mouse_pos();

	int cell_x = ((mouse_pos.x + cam_x) / currently_editing_rect.w) * currently_editing_sprites_rect.w;
	int cell_y = ((mouse_pos.y + cam_y) / currently_editing_rect.h) * currently_editing_sprites_rect.h;

	if (point_in_recti(mouse_pos, map_rect)) {
		if (api_mouse_btn(computer, MOUSE_BUTTON_LEFT)) {
			for (int i = 0; i < currently_editing_sprites_rect.h; i++) {
				for (int j = 0; j < currently_editing_sprites_rect.w; j++) {
					// TODO: make a function for this
					int index = (selected_spritesheet_index * SPRITES_PER_PAGE) + sprite_coords_to_index(currently_editing_sprites_rect.x + j, currently_editing_sprites_rect.y + i);
					computer->ram->map.layers[selected_layer].data[(cell_y + i) * MAP_WIDTH + (cell_x + j)] = (uint16_t)index;
				}
			}
		}
	
		if (api_mouse_btn(computer, MOUSE_BUTTON_RIGHT)) {
			for (int i = 0; i < currently_editing_sprites_rect.h; i++) {
				for (int j = 0; j < currently_editing_sprites_rect.w; j++) {
					computer->ram->map.layers[selected_layer].data[(cell_y + i) * MAP_WIDTH + (cell_x + j)] = 0;
				}
			}
		}
	}

	if (api_key(computer, KEY_A)) {
		cam_x -= move_speed;
	}
	if (api_key(computer, KEY_D)) {
		cam_x += move_speed;
	}
	if (api_key(computer, KEY_W)) {
		cam_y -= move_speed;
	}
	if (api_key(computer, KEY_S)) {
		cam_y += move_speed;
	}

	if (api_mouse_scrolled(computer, SCROLL_DOWN)) {
		zoom *= 2.0f;
	}
	if (api_mouse_scrolled(computer, SCROLL_UP)) {
		zoom *= 0.5f;
	}
}

void map_editor_draw(computer_t *computer) {
	api_rectf(computer, map_rect.x, map_rect.y, map_rect.w, map_rect.h, 0);

	// Draw only the visible portion so we're not drawing the entire map
	// yeah
	int map_x = cam_x / SPRITE_WIDTH;
	int map_y = cam_y / SPRITE_HEIGHT;
	// 81 so the screen is always filled

	for (int i = 0; i < MAP_LAYERS_AMOUNT; i++) {
		api_draw_map_layer(computer, i, -cam_x, -cam_y, map_x, map_y, 81, 60);
	}

	// Draw rect where mouse is
	vec2i_t mouse_pos = input_get_mouse_pos();

	if (point_in_recti(mouse_pos, map_rect)) {
		int rect_x = ((mouse_pos.x + cam_x) / currently_editing_rect.w) * currently_editing_rect.w - cam_x;
		int rect_y = ((mouse_pos.y + cam_y) / currently_editing_rect.h) * currently_editing_rect.h - cam_y;

		api_rect(computer, rect_x - 1, rect_y - 1, currently_editing_rect.w + 2, currently_editing_rect.h + 2, COLOR_WHITE);
	}

	draw_grid(computer);

	gui_outset_frame(computer->ram, (recti_t){0, 344, 640, 136});
	sprite_selector_draw(computer);

	gui_button(computer->ram, "Entities", (recti_t){2, SCREEN_HEIGHT - 5 * 16 - 2, 48, 16});

	// Layer buttons
	for (int i = 0; i < MAP_LAYERS_AMOUNT; i++) {
		char buffer[2];
		sprintf(buffer, "%d", i);

		recti_t rect = {
			.x = 2,
			.y = SCREEN_HEIGHT - 4 * 16 - 2 + i * 16,
			.w = 48,
			.h = 16
		};

		if (gui_button_ex(computer->ram, buffer, rect, selected_layer == i)) {
			selected_layer = i;
		}
	}
}