#include "sprite.h"

#include <stdio.h>

#include "../backend/input.h"
#include "../backend/gfx.h"
#include "../backend/gui.h"
#include "../common/mem.h"
#include "menu.h"
#include "shared.h"

#define COLOR_SQUARE_SIZE 8

// All GUI element rects and positions in one place
typedef struct layout {
	rect_t color_picker_rect;
	rect_t sprite_editor_rect;

	rect_t selected_color_rect;
	point_t selected_color_label_pos;

	rect_t selected_sprite_rect;
	point_t selected_sprite_label_pos;

	point_t color_key_button_pos;
	rect_t color_key_rect;

	point_t sprite_flags_start_pos;

	rect_t spritesheet_rect;
	point_t spritesheet_pages_start_pos;

	point_t tools_start_pos;
} layout_t;

#define SPRITE_EDITOR_WIDTH 256
#define SPRITE_EDITOR_HEIGHT 256

static const layout_t _layout = {
	.color_picker_rect = {{4, 388, 192, 88}},
	.sprite_editor_rect = {{192, 56, SPRITE_EDITOR_WIDTH, SPRITE_EDITOR_HEIGHT}},

	.selected_color_rect = {{4, 368, 16, 16}},
	.selected_color_label_pos = {24, 372},

	.selected_sprite_rect = {{4, 348, 16, 16}},
	.selected_sprite_label_pos = {24, 351},

	// .color_key_button_rect = RECT(622, 332, 12, 12),
	.color_key_button_pos = {588, 332},
	.color_key_rect = {{590, 334, 8, 8}},

	.sprite_flags_start_pos = {200, 332},

	.spritesheet_rect = {{200, 348, 384, 128}},
	.spritesheet_pages_start_pos = {588, 348},

	.tools_start_pos = {264, 38},
};

static uint8_t _selected_color = 0;

static point_t _change_start = {0};
static point_t _change_end = {0};

static point_t _min_reached_point = {0};
static point_t _max_reached_point = {0};

typedef struct change {
	rect_t region;
	surface_t before;
	// color_t *after;
} change_t;

// I need some kind of overlay for the sprite editor, so
// when you draw some stuff it will be on the overlay, then a change can be constructed
// by getting the before from the real memory, then commiting the overlay and then getting the real memory again

// This is also needed for drawing shapes like rectangle so there is a place to preview to
// Although with that it would need to be cleared every frame
// Or actually I can just draw it directly to the framebuffer until it is committed
// Final though the preview is drawn to the framebuffer, then when the mouse is released again the change object is made and the rect is first only drawn on the overlay as well

// Actually this won't work because when you draw freely it isn't guaranteed that
// everything drawn is within the start and end point
// the affected rect should get updated while drawing instead

typedef enum tool {
	TOOL_PENCIL,
	TOOL_LINE,
	TOOL_RECT,
	TOOL_RECTF,
	TOOL_ELLIPSE,
	TOOL_ELLIPSEF,
	TOOL_BUCKET,

	TOOL_COUNT,
} tool_t;

static tool_t _selected_tool = TOOL_PENCIL;

// TODO: before i commit
// I think that it is actually not necessary to track exactly which region to change, I can just save the currently_editing_rect to the undo stack
// And I can push the old state to the undo stack when I detect an action starting to happen instead of all the overlay business
// So I can remove the overlay
// And I can put the shape previews directly on the framebuffer
// by keeping a tool_state struct, setting it in update and drawing stuff in draw based on that
// yep that all sounds pretty good
// Changing the gfx stuff to take surfaces was still good because I need that for the spritesheet
// Actually for the redo stuff I also need to know what it looks like after so I still need to push onto the undo stack after the action
// but I could have a global variable of _current_change for that
// For now I'm actually gonna keep it

// For the map editor it would still be good to track the changed region dynamically since there is no 'currently editing' rect
// I think the dynamic tracking would still be possible without the overlay no wait nevermind

// Should actually not be accessed at all
static color_t _overlay_data[SPRITE_EDITOR_WIDTH * SPRITE_EDITOR_HEIGHT];
// Instead this should be accessed
static surface_t _overlay = {
	.data = _overlay_data,
	.width = 256,
	.height = 256,
};

// TODO: free this
static zinc_stack_t _undo_stack = {0};

#define UNDO_STACK_SIZE 64

void sprite_editor_init(computer_t *computer) {
	gfx_clear(_overlay, COLOR_NONE);

	stack_init(&_undo_stack, sizeof(change_t), UNDO_STACK_SIZE);
}

static point_t _color_index_to_pos(uint8_t color) {
	point_t pos = {0};

	if (color < 32) {
		pos.x = _layout.color_picker_rect.x + (color % 16) * COLOR_SQUARE_SIZE;
		pos.y = _layout.color_picker_rect.y + (color / 16) * COLOR_SQUARE_SIZE;
	} else {
		pos.x = _layout.color_picker_rect.x + ((color - 32) % 24) * COLOR_SQUARE_SIZE;
		pos.y = _layout.color_picker_rect.y + 2 * COLOR_SQUARE_SIZE + ((color - 32) / 24) * COLOR_SQUARE_SIZE;
	}

	return pos;
}

static uint8_t _pos_to_color_index(point_t pos) {
	int cell_x = (pos.x - _layout.color_picker_rect.x) / COLOR_SQUARE_SIZE;
	int cell_y = (pos.y - _layout.color_picker_rect.y) / COLOR_SQUARE_SIZE;

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

static point_t _editor_to_spritesheet_pos(point_t point) {
	return POINT(visible_rect.x + currently_editing_rect.x + point.x, visible_rect.y + currently_editing_rect.y + point.y);
}

static void _push_to_undo(computer_t *computer, rect_t region) {
	// Create the change
	change_t change = {0};
	change.region = region;
	// Allocate memory and copy changed region
	change.before.data = calloc(region.w * region.h, sizeof(color_t));
	change.before.width = region.w;
	change.before.height = region.h;
	gfx_copy_surface_rect(change.before, SPR_SURF(computer->ram->spritesheet.data), POINT(0, 0), region, COLOR_NONE);

	// Push onto the undo stack
	stack_push(&_undo_stack, &change);
}

static void _undo(computer_t *computer) {
	change_t change = {0};
	if (stack_pop(&_undo_stack, &change)) {
		// Copy changed region back to spritesheet
		gfx_copy_surface_rect(SPR_SURF(computer->ram->spritesheet.data), change.before, change.region.pos, RECT(0, 0, change.region.w, change.region.h), COLOR_NONE);
	
		free(change.before.data);
		change.before.data = NULL;
	}

}

void sprite_editor_update(computer_t *computer) {
	point_t mouse_pos = input_get_mouse_pos();
	
	sprite_selector_update(computer, SNAP_MODE_ZOOM, _layout.spritesheet_rect);

	if (point_in_rect(mouse_pos, _layout.color_picker_rect)) {
		if (input_mouse_button_held(MOUSE_BUTTON_LEFT)) {
			_selected_color = _pos_to_color_index(mouse_pos);
		}
	}

	// Undo
	if (input_key_held(KEY_LCTRL)) {
		if (input_key_pressed(KEY_Z)) {
			_undo(computer);
		}
	}

	if (point_in_rect(mouse_pos, _layout.sprite_editor_rect)) {
		point_t local_coord = {
			.x = (mouse_pos.x - _layout.sprite_editor_rect.x) / (_layout.sprite_editor_rect.w / currently_editing_rect.w),
			.y = (mouse_pos.y - _layout.sprite_editor_rect.y) / (_layout.sprite_editor_rect.h / currently_editing_rect.h),
		};

		if (input_mouse_button_pressed(MOUSE_BUTTON_LEFT)) {
			_change_start = local_coord;
		}

		// TODO: Maybe remove the overlay and dynamic tracking of changes and just render previews to framebuffer and copy the currently editing rect region to a change object
		switch (_selected_tool) {
			case (TOOL_PENCIL): {
				if (input_mouse_button_held(MOUSE_BUTTON_LEFT)) {
					_min_reached_point.x = MIN(_min_reached_point.x, local_coord.x);
					_min_reached_point.y = MIN(_min_reached_point.y, local_coord.y);
					_max_reached_point.x = MAX(_max_reached_point.x, local_coord.x);
					_max_reached_point.y = MAX(_max_reached_point.y, local_coord.y);

					if (input_key_held(KEY_LALT) || input_key_held(KEY_RALT)) {
						_selected_color = computer->ram->spritesheet.data[(visible_rect.y + currently_editing_rect.y + local_coord.y) * SPRITESHEET_WIDTH + (visible_rect.x + currently_editing_rect.x + local_coord.x)];
					}
		
					surf_set_pixel(_overlay, local_coord.x, local_coord.y, _selected_color);
				}

				if (input_mouse_button_held(MOUSE_BUTTON_RIGHT)) {
					// TODO: implement secondary color and use here instead of black
					surf_set_pixel(_overlay, local_coord.x, local_coord.y, COLOR_BLACK);
				}

				if (input_mouse_button_released(MOUSE_BUTTON_LEFT) || input_mouse_button_released(MOUSE_BUTTON_RIGHT)) {
					// Copy the affected part of the overlay to the undo stack and spritesheet
					rect_t changed_region = rect_from_2_points(_editor_to_spritesheet_pos(_min_reached_point), _editor_to_spritesheet_pos(_max_reached_point));
					changed_region.w++;
					changed_region.h++;
					_push_to_undo(computer, changed_region);

					// Copy overlay to spritesheet
					gfx_copy_surface_rect(SPR_SURF(computer->ram->spritesheet.data), _overlay, POINT(visible_rect.x + currently_editing_rect.x, visible_rect.y + currently_editing_rect.y), RECT(0, 0, currently_editing_rect.w, currently_editing_rect.h), COLOR_NONE);

					gfx_clear(_overlay, COLOR_NONE);
				}
		

				break;
			}

			case (TOOL_LINE): {
				gfx_clear(_overlay, COLOR_NONE);

				if (input_mouse_button_held(MOUSE_BUTTON_LEFT)) {
					_change_end = local_coord;
					gfx_draw_line(_overlay, _change_start, _change_end, _selected_color);
				}

				if (input_mouse_button_released(MOUSE_BUTTON_LEFT)) {
					rect_t changed_region = rect_from_2_points(_editor_to_spritesheet_pos(_change_start), _editor_to_spritesheet_pos(_change_end));
					changed_region.w++;
					changed_region.h++;
					_push_to_undo(computer, changed_region);

					// Actually commit the change
					gfx_draw_line(SPR_SURF(computer->ram->spritesheet.data), _editor_to_spritesheet_pos(_change_start), _editor_to_spritesheet_pos(_change_end), _selected_color);
				}

				break;
			}

			case (TOOL_RECT): {
				gfx_clear(_overlay, COLOR_NONE);

				if (input_mouse_button_held(MOUSE_BUTTON_LEFT)) {
					_change_end = local_coord;
					rect_t rect = rect_from_2_points(_change_start, _change_end);
					rect.w++;
					rect.h++;
					gfx_draw_rect(_overlay, rect, _selected_color);
				}

				if (input_mouse_button_released(MOUSE_BUTTON_LEFT)) {
					rect_t raw_rect = rect_from_2_points(_change_start, _change_end);
					rect_t rect = {
						.x = visible_rect.x + currently_editing_rect.x + raw_rect.x,
						.y = visible_rect.y + currently_editing_rect.y + raw_rect.y,
						.w = raw_rect.w + 1,
						.h = raw_rect.h + 1,
					};

					rect_t changed_region = rect_from_2_points(_editor_to_spritesheet_pos(_change_start), _editor_to_spritesheet_pos(_change_end));
					changed_region.w++;
					changed_region.h++;
					_push_to_undo(computer, changed_region);

					gfx_draw_rect(SPR_SURF(computer->ram->spritesheet.data), rect, _selected_color);
				}

				break;
			}

			case (TOOL_RECTF): {
				gfx_clear(_overlay, COLOR_NONE);

				if (input_mouse_button_held(MOUSE_BUTTON_LEFT)) {
					_change_end = local_coord;
					rect_t rect = rect_from_2_points(_change_start, _change_end);
					rect.w++;
					rect.h++;
					gfx_draw_filled_rect(_overlay, rect, _selected_color);
				}

				if (input_mouse_button_released(MOUSE_BUTTON_LEFT)) {
					rect_t raw_rect = rect_from_2_points(_change_start, _change_end);
					rect_t rect = {
						.x = visible_rect.x + currently_editing_rect.x + raw_rect.x,
						.y = visible_rect.y + currently_editing_rect.y + raw_rect.y,
						.w = raw_rect.w + 1,
						.h = raw_rect.h + 1,
					};

					rect_t changed_region = rect_from_2_points(_editor_to_spritesheet_pos(_change_start), _editor_to_spritesheet_pos(_change_end));
					changed_region.w++;
					changed_region.h++;
					_push_to_undo(computer, changed_region);

					gfx_draw_filled_rect(SPR_SURF(computer->ram->spritesheet.data), rect, _selected_color);
				}

				break;
			}
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
	surface_t fb_surf = FB_SURF(fb->data);

	// Spritesheet / sprite selector
	sprite_selector_draw(computer, _layout.spritesheet_rect, _layout.spritesheet_pages_start_pos);

	// Color picker frame
	gfx_draw_filled_rect(fb_surf, _layout.color_picker_rect, 0);

	// Draw colors
	for (int i = 0; i < PALETTE_SIZE - 8; i++) {
		point_t color_cell_pos = _color_index_to_pos(i);
		gfx_draw_filled_rect(fb_surf, RECT(color_cell_pos.x, color_cell_pos.y, COLOR_SQUARE_SIZE, COLOR_SQUARE_SIZE), i);
	}

	// Draw selected color square
	point_t selected_color_cell_pos = _color_index_to_pos(_selected_color);
	gfx_draw_rect(fb_surf, RECT(selected_color_cell_pos.x - 1, selected_color_cell_pos.y - 1, COLOR_SQUARE_SIZE + 2, COLOR_SQUARE_SIZE + 2), 15);

	// Sprite editor
	rect_t sprite_editing_rect = {
		.x = visible_rect.x + currently_editing_rect.x,
		.y = visible_rect.y + currently_editing_rect.y,
		.w = currently_editing_rect.w,
		.h = currently_editing_rect.h,
	};

	int scale = _layout.sprite_editor_rect.w / currently_editing_rect.w;
	rect_t real_editor_rect = {
		.x = _layout.sprite_editor_rect.x,
		.y = _layout.sprite_editor_rect.y,
		.w = currently_editing_rect.w * scale,
		.h = currently_editing_rect.h * scale,
	};
	gfx_draw_filled_rect(fb_surf, _layout.sprite_editor_rect, 151);
	gfx_draw_spritesheet_pro(computer->ram, sprite_editing_rect, real_editor_rect, COLOR_NONE);

	// Draw the overlay
	gfx_draw_surface_pro(fb, _overlay, RECT(0, 0, currently_editing_rect.w, currently_editing_rect.h), real_editor_rect, COLOR_NONE);

	// Selected color
	char buffer[32];
	gfx_draw_filled_rect(fb_surf, _layout.selected_color_rect, _selected_color);
	sprintf(buffer, "#%03d\n", _selected_color);
	gui_draw_text(computer->ram, GUI_FONT_INDEX, buffer, _layout.selected_color_label_pos, computer->ram->skin.font_color);

	// gui_draw_text(computer->ram, 2, buffer, _layout.selected_color_label_pos, COLOR_NONE); // Testing not passing a color
	
	// Selected sprite preview
	gfx_draw_spritesheet_pro(computer->ram, currently_editing_rect, _layout.selected_sprite_rect, COLOR_NONE); // TODO: fix so it adds the other rects to currently_editing_rect
	sprintf(buffer, "#%04d\n", get_selected_sprite_index());
	gui_draw_text(computer->ram, GUI_FONT_INDEX, buffer, _layout.selected_sprite_label_pos, computer->ram->skin.font_color);

	// Sprite flags and color key
	sprite_t *selected_sprite = &computer->ram->sprites[get_selected_sprite_index()];

	// Still using the macro because it is probably safer
	for (int i = 0; i < SPRITE_FLAGS_SIZE; i++) {
		bool set = selected_sprite->flags & (1U << i);

		point_t pos = button_array_get_pos(&skin_layout.sprite_flag_buttons, _layout.sprite_flags_start_pos, i);
		button_t button = button_array_get(&skin_layout.sprite_flag_buttons, i);
		
		set = gui_toggle_button(computer->ram, pos, button, set);

		if (set) {
			selected_sprite->flags |= (1U << i);
		} else {
			selected_sprite->flags &= ~(1U << i);
		}
	}

	// Color key
	if (gui_button(computer->ram, _layout.color_key_button_pos, skin_layout.color_key_button, false)) {
		selected_sprite->color_key = _selected_color;
	}
	gfx_draw_filled_rect(fb_surf, _layout.color_key_rect, selected_sprite->color_key);

	// Tools
	for (int i = 0; i < skin_layout.sprite_tool_buttons.amount; i++) {
		point_t pos = button_array_get_pos(&skin_layout.sprite_tool_buttons, _layout.tools_start_pos, i);
		button_t button = button_array_get(&skin_layout.sprite_tool_buttons, i);

		if (gui_button(computer->ram, pos, button, i == _selected_tool)) {
			_selected_tool = i;
		}
	}
}