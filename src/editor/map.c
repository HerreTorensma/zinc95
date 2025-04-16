#include "map.h"
#include "menu.h"

#include "../api/api.h"
#include "../backend/input.h"
#include "../backend/backend.h"
#include "../util/util.h"
#include "shared.h"

static rect_t map_rect = {0};

void map_editor_init(computer_t *computer) {
	map_rect = workspace_rect;
	map_rect.h -= 136;
}

void map_editor_update(computer_t *computer) {
	sprite_selector_update(computer, SNAP_MODE_ZOOM);

	int x, y;
	get_mouse_pos(&x, &y);

	int cell_x = (x / currently_editing_rect.w) * currently_editing_sprites_rect.w;
	int cell_y = (y / currently_editing_rect.h) * currently_editing_sprites_rect.h;

	if (point_in_rect(x, y, map_rect)) {
		if (api_mouse_btn(computer, MOUSE_BUTTON_LEFT)) {
			for (int i = 0; i < currently_editing_sprites_rect.h; i++) {
				for (int j = 0; j < currently_editing_sprites_rect.w; j++) {
					int index = (selected_spritesheet_index * SPRITES_PER_PAGE) + sprite_coords_to_index(currently_editing_sprites_rect.x + j, currently_editing_sprites_rect.y + i);
					computer->ram->map.layers[0].data[(cell_y + i) * MAP_WIDTH + (cell_x + j)] = (uint16_t)index;
				}
			}
		}
	
		if (api_mouse_btn(computer, MOUSE_BUTTON_RIGHT)) {
			for (int i = 0; i < currently_editing_sprites_rect.h; i++) {
				for (int j = 0; j < currently_editing_sprites_rect.w; j++) {
					computer->ram->map.layers[0].data[(cell_y + i) * MAP_WIDTH + (cell_x + j)] = 0;
				}
			}
		}
	}
}

void map_editor_draw(computer_t *computer) {
	api_rectf(computer, map_rect.x, map_rect.y, map_rect.w, map_rect.h, 0);

	// api_draw_map_layer(computer, 0, 0, 0, map_rect.x, map_rect.y, 80, 40);
	api_draw_map_layer(computer, 0, 0, 0, 0, 0, 80, 60);

	// Draw rect where mouse is
	int x, y;
	get_mouse_pos(&x, &y);
	
	if (point_in_rect(x, y, map_rect)) {
		int rect_x = (x / currently_editing_rect.w) * currently_editing_rect.w;
		int rect_y = (y / currently_editing_rect.h) * currently_editing_rect.h;
		api_rect(computer, rect_x - 1, rect_y - 1, currently_editing_rect.w + 2, currently_editing_rect.h + 2, COLOR_WHITE);
	}

	draw_out_frame(computer, (rect_t){0, 344, 640, 136});
	sprite_selector_draw(computer);
}