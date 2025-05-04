#include "sprite.h"

#include <stdio.h>

#include "../backend/input.h"
#include "../backend/gfx.h"
#include "../backend/gui.h"
#include "menu.h"
#include "shared.h"

#define COLOR_SQUARE_SIZE 8

static rect_t color_picker_rect = {0};
static rect_t sprite_editor_rect = {0};

// All GUI element rects and positions in one place
typedef struct layout {
	rect_t color_picker_rect;
	rect_t sprite_editor_rect;

	rect_t selected_color_rect;
	rect_t selected_color_label_rect;

	rect_t selected_sprite_rect;
	rect_t selected_sprite_label_rect;

	rect_t color_key_button_rect;
	rect_t color_key_label_rect;

	point_t sprite_flags_start_pos;

	rect_t spritesheet_rect;
	point_t spritesheet_pages_start_pos;
} layout_t;

static layout_t layout = {0};

static uint8_t selected_color = 0;

void init_layout() {

}

void sprite_editor_init(computer_t *computer) {
	// color_picker_rect = RECT(4, 388, 192, 88);
	color_picker_rect = rect_anchor_bottom(workspace_rect, RECT(GUI_BORDER_WIDTH*2, 0, 24*COLOR_SQUARE_SIZE, 11*COLOR_SQUARE_SIZE), GUI_BORDER_WIDTH*2);
	sprite_editor_rect = RECT(192, 56, 256, 256);
	// sprite_editor_rect = rect_put_right(color_picker_rect, RECT())

	// color_picker_rect = rect_p
}

static point_t color_index_to_pos(uint8_t color) {
	point_t pos = {0};

	if (color < 32) {
		pos.x = color_picker_rect.x + (color % 16) * COLOR_SQUARE_SIZE;
		pos.y = color_picker_rect.y + (color / 16) * COLOR_SQUARE_SIZE;
	} else {
		pos.x = color_picker_rect.x + ((color - 32) % 24) * COLOR_SQUARE_SIZE;
		pos.y = color_picker_rect.y + 2 * COLOR_SQUARE_SIZE + ((color - 32) / 24) * COLOR_SQUARE_SIZE;
	}

	return pos;
}

static uint8_t pos_to_color_index(point_t pos) {
	int cell_x = (pos.x - color_picker_rect.x) / COLOR_SQUARE_SIZE;
	int cell_y = (pos.y - color_picker_rect.y) / COLOR_SQUARE_SIZE;

	if ((cell_y == 0 || cell_y == 1) && cell_x >= 16) {
		return 0;
	}

	if (cell_y == 0) {
		return cell_x;
	} else if (cell_y == 1) {
		return 16 + cell_x;
	} else {
		return 32 + ((cell_y - 2) * 24 + cell_x);
	}
}

void sprite_editor_update(computer_t *computer) {
	point_t mouse_pos = input_get_mouse_pos();
	
	sprite_selector_update(computer, SNAP_MODE_ZOOM);

	if (point_in_rect(mouse_pos, color_picker_rect)) {
		if (input_mouse_button_held(MOUSE_BUTTON_LEFT)) {
			selected_color = pos_to_color_index(mouse_pos);
		}
	}

	if (point_in_rect(mouse_pos, sprite_editor_rect)) {
		int cell_x = (mouse_pos.x - sprite_editor_rect.x) / (sprite_editor_rect.w / currently_editing_rect.w);
		int cell_y = (mouse_pos.y - sprite_editor_rect.y) / (sprite_editor_rect.h / currently_editing_rect.h);
		
		if (input_mouse_button_held(MOUSE_BUTTON_LEFT)) {
			if (input_key_held(KEY_LALT) || input_key_held(KEY_RALT)) {
				// TODO: make a function for this
				selected_color = computer->ram->spritesheet.data[(visible_rect.y + currently_editing_rect.y + cell_y) * SPRITESHEET_WIDTH + (visible_rect.x + currently_editing_rect.x + cell_x)];
			}

			computer->ram->spritesheet.data[(visible_rect.y + currently_editing_rect.y + cell_y) * SPRITESHEET_WIDTH + (visible_rect.x + currently_editing_rect.x + cell_x)] = selected_color;

		}

		if (input_mouse_button_held(MOUSE_BUTTON_RIGHT)) {
			computer->ram->spritesheet.data[(visible_rect.y + currently_editing_rect.y + cell_y) * SPRITESHEET_WIDTH + (visible_rect.x + currently_editing_rect.x + cell_x)] = 0;
		}
	}

	// Delete sprite
	if (input_key_pressed(KEY_DELETE)) {
		// TODO: add visible rect stuff
		for (int y = currently_editing_rect.y; y < currently_editing_rect.y + currently_editing_rect.h; y++) {
			for (int x = currently_editing_rect.x; x < currently_editing_rect.x + currently_editing_rect.w; x++) {
				computer->ram->spritesheet.data[y * SPRITESHEET_WIDTH + x] = 0;
			}
		}

		// TODO: clear this stuff for every sprite in the selection
		computer->ram->sprites[get_selected_sprite_index()].color_key = 0;
		computer->ram->sprites[get_selected_sprite_index()].flags = 0U;
	}
}

void sprite_editor_draw(computer_t *computer) {
	framebuffer_t *fb = &computer->ram->framebuffer;

	// Spritesheet / sprite selector
	sprite_selector_draw(computer);

	// Color picker frame
	gui_inset_frame(computer->ram, color_picker_rect);
	gfx_draw_filled_rect(fb, color_picker_rect, 0);

	// Draw colors
	for (int i = 0; i < PALETTE_SIZE - 8; i++) {
		point_t color_cell_pos = color_index_to_pos(i);
		gfx_draw_filled_rect(fb, RECT(color_cell_pos.x, color_cell_pos.y, COLOR_SQUARE_SIZE, COLOR_SQUARE_SIZE), i);
	}

	// Draw selected color square
	point_t selected_color_cell_pos = color_index_to_pos(selected_color);
	gfx_draw_rect(fb, RECT(selected_color_cell_pos.x - 1, selected_color_cell_pos.y - 1, COLOR_SQUARE_SIZE + 2, COLOR_SQUARE_SIZE + 2), 15);

	// Sprite editor
	gui_inset_frame(computer->ram, sprite_editor_rect);
	rect_t sprite_editing_rect = {
		.x = visible_rect.x + currently_editing_rect.x,
		.y = visible_rect.y + currently_editing_rect.y,
		.w = currently_editing_rect.w,
		.h = currently_editing_rect.h,
	};

	int scale = sprite_editor_rect.w / currently_editing_rect.w;
	rect_t real_editor_rect = {
		.x = sprite_editor_rect.x,
		.y = sprite_editor_rect.y,
		.w = currently_editing_rect.w * scale,
		.h = currently_editing_rect.h * scale,
	};
	gfx_draw_filled_rect(fb, sprite_editor_rect, 151);
	gfx_draw_spritesheet_pro(computer->ram, sprite_editing_rect, real_editor_rect, COLOR_NONE);

	// Selected color
	rect_t selected_color_rect = rect_put_above(color_picker_rect, RECT(0, 0, 16, 16), 0);
	char buffer[32];
	gui_inset_frame(computer->ram, RECT(color_picker_rect.x, color_picker_rect.y - 20, 16, 16));
	gfx_draw_filled_rect(fb, RECT(color_picker_rect.x, color_picker_rect.y - 20, 16, 16), selected_color);
	sprintf(buffer, "#%03d\n", selected_color);
	gui_draw_text(computer->ram, 0, buffer, POINT(color_picker_rect.x + 20, color_picker_rect.y - 16), computer->ram->gui_colors.text);
	
	// Selected sprite preview
	rect_t selected_sprite_rect = rect_put_above(selected_color_rect, RECT(0, 0, 16, 16), 0);
	gui_inset_frame(computer->ram, RECT(color_picker_rect.x, color_picker_rect.y - 40, 16, 16));
	gfx_draw_sprites(computer->ram, get_selected_sprite_index(), POINT(color_picker_rect.x, color_picker_rect.y - 40), 1, 1);
	sprintf(buffer, "#%04d\n", get_selected_sprite_index());
	gui_draw_text(computer->ram, 0, buffer, POINT(color_picker_rect.x + 20, color_picker_rect.y - 36), computer->ram->gui_colors.text);

	// Sprite flags and color key
	sprite_t *selected_sprite = &computer->ram->sprites[get_selected_sprite_index()];
	for (int i = 0; i < SPRITE_FLAGS_SIZE; i++) {
		sprintf(buffer, "%c", i < 10 ? '0' + i : 'a' + i - 10);
		
		bool set = selected_sprite->flags & (1U << i);
		set = gui_toggle_button(computer->ram, buffer, RECT(spritesheet_rect.x + i * 12, spritesheet_rect.y - 4 - 12, 12, 12), set);

		// TODO: update for every selected sprite, not just top left
		if (set) {
			selected_sprite->flags |= (1U << i);
		} else {
			selected_sprite->flags &= ~(1U << i);
		}
	}

	// Color key
	if (gui_button(computer->ram, "", RECT(SCREEN_WIDTH - 2 - 12 - 4, spritesheet_rect.y - 12 - 4, 12, 12))) {
		selected_sprite->color_key = selected_color;
	}

	gfx_draw_filled_rect(fb, RECT(SCREEN_WIDTH - 2 - 12 - 4 + 2, spritesheet_rect.y - 12 - 4 + 2, 8, 8), selected_sprite->color_key);
	gui_draw_text(computer->ram, 2, "Key:", POINT(spritesheet_rect.x + spritesheet_rect.w + 4 + 2, spritesheet_rect.y - 12 - 4 + 2), computer->ram->gui_colors.text);
}