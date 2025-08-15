#include "shared.h"

#include <stdio.h>
#include <math.h>

#include "../api/api.h"
#include "../backend/input.h"
#include "../backend/gfx.h"
#include "../backend/gui.h"

// TODO: refactor this because this is perhaps the worst code in the project

// TODO: look into making these static or something
rect_t visible_rect = {0};

// Rect in pixels
rect_t currently_editing_rect = {0};

// Rect in sprites
rect_t currently_editing_sprites_rect = {0};

int selected_sprite_index_offset = 0;
int selected_spritesheet_index = 0;

static void _set_selected_spritesheet_index(int index) {
	selected_spritesheet_index = index;
	visible_rect = (rect_t){
		.x = 0,
		.y = selected_spritesheet_index * SPRITESHEET_PAGE_HEIGHT,
		.w = SPRITESHEET_PAGE_WIDTH,
		.h = SPRITESHEET_PAGE_HEIGHT,
	};
}

void sprite_selector_init(computer_t *computer) {
	visible_rect = (rect_t){
		.x = 0,
		.y = selected_spritesheet_index * SPRITESHEET_PAGE_HEIGHT,
		.w = SPRITESHEET_PAGE_WIDTH,
		.h = SPRITESHEET_PAGE_HEIGHT,
	};
	
	currently_editing_rect = (rect_t){
		.x = 0,
		.y = 0,
		.w = SPRITE_WIDTH,
		.h = SPRITE_HEIGHT,
	};
}

// TODO: make this a setter for currently_editing_rect
static void _update_currently_editing_sprites_rect(sprite_select_snap_mode_t snap_mode) {
	currently_editing_sprites_rect.w = currently_editing_rect.w / SPRITE_WIDTH;
	currently_editing_sprites_rect.h = currently_editing_rect.h / SPRITE_HEIGHT;

	if (snap_mode == SNAP_MODE_ZOOM) {
		// Update position as well
		currently_editing_rect.x = (currently_editing_rect.x / currently_editing_rect.w) * currently_editing_rect.w;
		currently_editing_rect.y = (currently_editing_rect.y / currently_editing_rect.h) * currently_editing_rect.h;

		// Didn't quite work but might look into later
		// currently_editing_rect.x = (int)(roundf((float)currently_editing_rect.x / (float)currently_editing_rect.w)) * currently_editing_rect.w;
		// currently_editing_rect.y = (int)(roundf((float)currently_editing_rect.y / (float)currently_editing_rect.h)) * currently_editing_rect.h;

		currently_editing_sprites_rect.x = currently_editing_rect.x / SPRITE_WIDTH;
		currently_editing_sprites_rect.y = currently_editing_rect.y / SPRITE_HEIGHT;
		
		selected_sprite_index_offset = currently_editing_sprites_rect.y * (SPRITESHEET_WIDTH / SPRITE_WIDTH) + currently_editing_sprites_rect.x;
	}
}

// This whole function is kind of a mess and I should probably rewrite it at some point
void sprite_selector_update(computer_t *computer, sprite_select_snap_mode_t snap_mode, rect_t spritesheet_rect) {
	point_t mouse_pos = input_get_mouse_pos();

	if (point_in_rect(mouse_pos, spritesheet_rect)) {
		if (input_key_pressed(KEY_MINUS) || input_mouse_scrolled(SCROLL_DIR_UP)) {
			currently_editing_rect.w -= SPRITE_WIDTH;
			currently_editing_rect.h -= SPRITE_HEIGHT;
			
			if (currently_editing_rect.w <= 0) {
				currently_editing_rect.w = SPRITE_WIDTH;
			}
			if (currently_editing_rect.h <= 0) {
				currently_editing_rect.h = SPRITE_HEIGHT;
			}
		}

		if (input_key_pressed(KEY_EQUALS) || input_mouse_scrolled(SCROLL_DIR_DOWN)) {
			currently_editing_rect.w += SPRITE_WIDTH;
			currently_editing_rect.h += SPRITE_HEIGHT;
		}

		if (input_mouse_button_held(MOUSE_BUTTON_LEFT)) {
			if (snap_mode == SNAP_MODE_SPRITE) {
				int adjusted_position_x = mouse_pos.x - spritesheet_rect.x;
				int adjusted_position_y = mouse_pos.y - spritesheet_rect.y;

				currently_editing_sprites_rect.x = adjusted_position_x / SPRITE_WIDTH;
				currently_editing_sprites_rect.y = adjusted_position_y / SPRITE_HEIGHT;
	
				currently_editing_rect.x = currently_editing_sprites_rect.x * SPRITE_WIDTH;
				currently_editing_rect.y = currently_editing_sprites_rect.y * SPRITE_HEIGHT;
				
				selected_sprite_index_offset = currently_editing_sprites_rect.y * (SPRITESHEET_WIDTH / SPRITE_WIDTH) + currently_editing_sprites_rect.x;
			} else if (snap_mode == SNAP_MODE_FREE) {
				// Free movement

				// TODO: update currently_editing_sprites_rect
				currently_editing_rect.x = mouse_pos.x - spritesheet_rect.x;
				currently_editing_rect.y = mouse_pos.y - spritesheet_rect.y;
			} else if (snap_mode == SNAP_MODE_ZOOM) {
				int adjusted_position_x = mouse_pos.x - spritesheet_rect.x;
				int adjusted_position_y = mouse_pos.y - spritesheet_rect.y;

				int cell_x = adjusted_position_x / currently_editing_rect.w;
				int cell_y = adjusted_position_y / currently_editing_rect.h;
	
				currently_editing_rect.x = cell_x * currently_editing_rect.w;
				currently_editing_rect.y = cell_y * currently_editing_rect.h;
				
				currently_editing_sprites_rect.x = currently_editing_rect.x / SPRITE_WIDTH;
				currently_editing_sprites_rect.y = currently_editing_rect.y / SPRITE_HEIGHT;
				
				selected_sprite_index_offset = currently_editing_sprites_rect.y * (SPRITESHEET_WIDTH / SPRITE_WIDTH) + currently_editing_sprites_rect.x;
			}
		}
	}

	_update_currently_editing_sprites_rect(snap_mode);
}

void sprite_selector_draw(computer_t *computer, rect_t spritesheet_rect, point_t page_buttons_pos) {
	// gui_inset_frame(computer->ram, spritesheet_rect);
	gfx_draw_spritesheet_rect(computer->ram, spritesheet_rect.pos, visible_rect, COLOR_NONE);

	for (int i = 0; i < skin_layout.spritesheet_page_buttons.amount; i++) {
		point_t pos = button_array_get_pos(&skin_layout.spritesheet_page_buttons, page_buttons_pos, i);
		button_t button = button_array_get(&skin_layout.spritesheet_page_buttons, i);

		if (gui_button(computer->ram, pos, button, selected_spritesheet_index == i)) {
			_set_selected_spritesheet_index(i);
		}
	}

	/*
	for (int y = 0; y < 8; y++) {
		for (int x = 0; x < 2; x++) {
			rect_t rect = {
				.x = spritesheet_rect.x + spritesheet_rect.w + 4 + x * 24,
				.y = spritesheet_rect.y + y * 16,
				.w = 24,
				.h = 16,
			};

			int index = y * 2 + x;
	
			char buffer[3];
			sprintf(buffer, "%d", index + 1);
			
			if (gui_button_ex(computer, buffer, rect, selected_spritesheet_index == index)) {
				_set_selected_spritesheet_index(index);
			}
		}
	}
	*/

	gfx_draw_rect(FB_SURF(computer->ram->framebuffer.data), RECT(spritesheet_rect.x + currently_editing_rect.x - 1, spritesheet_rect.y + currently_editing_rect.y - 1, currently_editing_rect.w + 2, currently_editing_rect.h + 2), 15);
}

int get_selected_sprite_index() {
	return (selected_spritesheet_index * SPRITES_PER_PAGE) + selected_sprite_index_offset;
}

int sprite_coords_to_index(int x, int y) {
	return y * (SPRITESHEET_WIDTH / SPRITE_WIDTH) + x;
}