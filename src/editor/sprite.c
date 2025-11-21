// Thoughts
// If no user made selection is active should the selection just be the in_frame_rect ???
// I also want to allow free movement like the map editor, like two modes of movement

#include "sprite.h"

#include <stdio.h>

#include "../backend/input.h"
#include "../backend/gfx.h"
#include "../backend/gui.h"
#include "../common/mem.h"
#include "shared.h"

#define COLOR_SQUARE_SIZE 8

// All GUI element rects and positions in one place
typedef struct layout {
	rect_t color_picker_rect;
	rect_t sprite_editor_focus_rect;
	rect_t sprite_editor_full_rect;

	rect_t selected_color_rect;
	point_t selected_color_label_pos;
	rect_t secondary_selected_color_rect;

	rect_t selected_sprite_rect;
	point_t selected_sprite_label_pos;

	point_t color_key_button_pos;
	rect_t color_key_rect;

	point_t sprite_flags_start_pos;

	point_t sprite_selector_pos;
	point_t sprite_selector_buttons_start_pos;

	point_t tools_start_pos;
} layout_t;

#define SPRITE_EDITOR_WIDTH 256
#define SPRITE_EDITOR_HEIGHT 256

static const layout_t _layout = {
	.color_picker_rect = {{4, 388, 192, 88}},
	.sprite_editor_focus_rect = {{192, 46, SPRITE_EDITOR_WIDTH, SPRITE_EDITOR_HEIGHT}},
	.sprite_editor_full_rect = {{0, 20, 620, 308}},

	.selected_color_rect = {{4, 368, 16, 16}},
	.selected_color_label_pos = {24, 372},
	.secondary_selected_color_rect = {{180, 368, 16, 16}},

	.selected_sprite_rect = {{4, 348, 16, 16}},
	.selected_sprite_label_pos = {24, 351},

	// .color_key_button_rect = RECT(622, 332, 12, 12),
	.color_key_button_pos = {588, 332},
	.color_key_rect = {{590, 334, 8, 8}},

	.sprite_flags_start_pos = {200, 332},

	.sprite_selector_pos = {200, 348},
	.sprite_selector_buttons_start_pos = {588, 348},

	.tools_start_pos = {622, 34},
};

static color_t _selected_color = 0;
static color_t _secondary_selected_color = 0;

static point_t _change_start = {0};
static point_t _change_end = {0};

static point_t _min_reached_point = {0};
static point_t _max_reached_point = {0};

static point_t _selection_start = {0};
static point_t _selection_end = {0};
static bool _selection_active = false;

static camera_t _camera = {
	// Put camera at the center of the screen
	.pos = (point_t){
		.x = 0,
		.y = 0,
	},
	.zoom = 1.0f,
	.screen_origin = POINT(192, 46), // sprite_editor_focus_rect.pos
};

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
	TOOL_SELECT,
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
static color_t _overlay_data[SPRITESHEET_WIDTH * SPRITESHEET_HEIGHT];
// Instead this should be accessed
static surface_t _overlay_surf = {
	.data = _overlay_data,
	.width = SPRITESHEET_WIDTH,
	.height = SPRITESHEET_HEIGHT,
};

// TODO: use
static color_t _selection_surface_data[SPRITESHEET_WIDTH * SPRITESHEET_HEIGHT];
static surface_t _selection_surf = {
	.data = _selection_surface_data,
	.width = SPRITESHEET_WIDTH,
	.height = SPRITESHEET_HEIGHT,
};

// TODO: free this
// TODO: use my array implementation for this maybe
static zinc_stack_t _undo_stack = {0};

#define UNDO_STACK_SIZE 64

void sprite_editor_init(computer_t *computer) {
	gfx_clear(_overlay_surf, COLOR_NONE);

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

static void _push_to_undo(computer_t *computer, rect_t region) {
	// Create the change
	change_t change = {0};
	change.region = region;
	// Allocate memory and copy changed region
	change.before.data = heap_alloc((region.w * region.h) * sizeof(color_t));
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

// Calculate it here so I don't need to keep a selection global updated
static rect_t _get_selection() {
	rect_t selection = rect_from_2_points(_selection_start, _selection_end);
	selection.w++;
	selection.h++;

	return selection;
}

// Move the rect
static void _move_rect(surface_t surf, rect_t rect, point_t new_pos) {
	surface_t temp_surface = {
		.data = temp_alloc((rect.w * rect.h) * sizeof(color_t)),
		.width = rect.w,
		.height = rect.h,
	};

	gfx_copy_surface_rect(temp_surface, surf, POINT(0, 0), rect, COLOR_NONE);

	// Erase
	gfx_draw_filled_rect(surf, rect, COLOR_NONE);

	// Copy back
	gfx_copy_surface_rect(surf, temp_surface, new_pos, RECT(0, 0, rect.w, rect.h), COLOR_NONE);
}

static void _commit_overlay(computer_t *computer) {
	gfx_copy_surface_rect(SPR_SURF(computer->ram->spritesheet.data), _overlay_surf, POINT(0, 0), RECT(0, 0, SPRITESHEET_WIDTH, SPRITESHEET_HEIGHT), COLOR_NONE);
	gfx_clear(_overlay_surf, COLOR_NONE);
}

static void _tool_select(computer_t *computer, point_t spritesheet_coord_under_mouse) {
	if (_selection_active) {
		rect_t selection = _get_selection();
		
		// TODO: use the PRESS_OR_LONG_PRESS macro or whatever it was
		if (input_key_pressed(KEY_LEFT)) {
			_move_rect(_overlay_surf, selection, POINT(selection.x - 1, selection.y));
			_selection_start.x--;
			_selection_end.x--;
		}
		
		if (input_key_pressed(KEY_RIGHT)) {
			_move_rect(_overlay_surf, selection, POINT(selection.x + 1, selection.y));
			_selection_start.x++;
			_selection_end.x++;
		}

		if (input_key_pressed(KEY_UP)) {
			_move_rect(_overlay_surf, selection, POINT(selection.x, selection.y - 1));
			_selection_start.y--;
			_selection_end.y--;
		}
		
		if (input_key_pressed(KEY_DOWN)) {
			_move_rect(_overlay_surf, selection, POINT(selection.x, selection.y + 1));
			_selection_start.y++;
			_selection_end.y++;
		}
	}

	if (input_mouse_button_pressed(MOUSE_BUTTON_LEFT)) {
		_selection_start = spritesheet_coord_under_mouse;
	}

	if (input_mouse_button_held(MOUSE_BUTTON_LEFT)) {
		_selection_end = spritesheet_coord_under_mouse;
	}

	if (input_mouse_button_released(MOUSE_BUTTON_LEFT)) {
		_selection_end = spritesheet_coord_under_mouse;

		// Commit if it was already active so there is a clean slate
		if (_selection_active) {
			_commit_overlay(computer);
		}

		_selection_active = true;
		if (_selection_start.x == _selection_end.x && _selection_start.y == _selection_end.y) {
			_selection_active = false;

			// Commit overlay to spritesheet
			_commit_overlay(computer);
			return;
		}
		
		rect_t selection = _get_selection();
		
		// Copy to overlay
		gfx_copy_surface_rect(_overlay_surf, SPR_SURF(computer->ram->spritesheet.data), selection.pos, selection, COLOR_NONE);
		// gfx_copy_surface_rect(_selection_surf, SPR_SURF(computer->ram->spritesheet.data), selection.pos, selection, COLOR_NONE);

		// Delete from spritesheet
		gfx_draw_filled_rect(SPR_SURF(computer->ram->spritesheet.data), selection, COLOR_BLACK);
	}
}

static void _tool_pencil(computer_t *computer, point_t spritesheet_coord_under_mouse) {
	// TODO: interpolate between points (draw line) so you don't get gaps between pixels when you draw really fast

	if (input_mouse_button_held(MOUSE_BUTTON_LEFT)) {
		_min_reached_point.x = MIN(_min_reached_point.x, spritesheet_coord_under_mouse.x);
		_min_reached_point.y = MIN(_min_reached_point.y, spritesheet_coord_under_mouse.y);
		_max_reached_point.x = MAX(_max_reached_point.x, spritesheet_coord_under_mouse.x);
		_max_reached_point.y = MAX(_max_reached_point.y, spritesheet_coord_under_mouse.y);

		surf_set_pixel(_overlay_surf, spritesheet_coord_under_mouse.x, spritesheet_coord_under_mouse.y, _selected_color);
	}

	if (input_mouse_button_held(MOUSE_BUTTON_RIGHT)) {
		// TODO: implement this secondary selected color for the other tools as well
		// TODO: should this also affect the change region and all that? yes probably
		surf_set_pixel(_overlay_surf, spritesheet_coord_under_mouse.x, spritesheet_coord_under_mouse.y, _secondary_selected_color);
	}

	if (input_mouse_button_released(MOUSE_BUTTON_LEFT) || input_mouse_button_released(MOUSE_BUTTON_RIGHT)) {
		// Copy the affected part of the overlay to the undo stack and spritesheet
		rect_t changed_region = rect_from_2_points(_min_reached_point, _max_reached_point);
		changed_region.w++;
		changed_region.h++;
		_push_to_undo(computer, changed_region);

		// Copy entire overlay instead
		_commit_overlay(computer);
	}
}

static void _tool_line(computer_t *computer, point_t spritesheet_coord_under_mouse) {
	gfx_clear(_overlay_surf, COLOR_NONE);

	if (input_mouse_button_held(MOUSE_BUTTON_LEFT)) {
		_change_end = spritesheet_coord_under_mouse;
		gfx_draw_line(_overlay_surf, _change_start, _change_end, _selected_color);
	}

	if (input_mouse_button_released(MOUSE_BUTTON_LEFT)) {
		rect_t changed_region = rect_from_2_points(_change_start, _change_end);
		changed_region.w++;
		changed_region.h++;
		_push_to_undo(computer, changed_region);

		// Actually commit the change
		gfx_draw_line(SPR_SURF(computer->ram->spritesheet.data), _change_start, _change_end, _selected_color);
	}
}

static void _tool_rect(computer_t *computer, point_t spritesheet_coord_under_mouse) {
	gfx_clear(_overlay_surf, COLOR_NONE);

	if (input_mouse_button_held(MOUSE_BUTTON_LEFT)) {
		_change_end = spritesheet_coord_under_mouse;
		rect_t rect = rect_from_2_points(_change_start, _change_end);
		rect.w++;
		rect.h++;
		gfx_draw_rect(_overlay_surf, rect, _selected_color);
	}

	if (input_mouse_button_released(MOUSE_BUTTON_LEFT)) {
		rect_t changed_region = rect_from_2_points(_change_start, _change_end);
		changed_region.w++;
		changed_region.h++;
		_push_to_undo(computer, changed_region);

		gfx_draw_rect(SPR_SURF(computer->ram->spritesheet.data), changed_region, _selected_color);
	}
}

static void _tool_rectf(computer_t *computer, point_t spritesheet_coord_under_mouse) {
	gfx_clear(_overlay_surf, COLOR_NONE);

	if (input_mouse_button_held(MOUSE_BUTTON_LEFT)) {
		_change_end = spritesheet_coord_under_mouse;
		rect_t rect = rect_from_2_points(_change_start, _change_end);

		rect.w++;
		rect.h++;
		gfx_draw_filled_rect(_overlay_surf, rect, _selected_color);
	}

	if (input_mouse_button_released(MOUSE_BUTTON_LEFT)) {
		rect_t changed_region = rect_from_2_points(_change_start, _change_end);
		changed_region.w++;
		changed_region.h++;
		_push_to_undo(computer, changed_region);

		gfx_draw_filled_rect(SPR_SURF(computer->ram->spritesheet.data), changed_region, _selected_color);
	}
}

static void _tool_ellipse(computer_t *computer, point_t spritesheet_coord_under_mouse) {
	gfx_clear(_overlay_surf, COLOR_NONE);
	
	if (input_mouse_button_held(MOUSE_BUTTON_LEFT)) {
		_change_end = spritesheet_coord_under_mouse;
		rect_t rect = rect_from_2_points(_change_start, _change_end);
		rect.w++;
		rect.h++;
		gfx_draw_ellipse(_overlay_surf, rect, _selected_color);
	}

	if (input_mouse_button_released(MOUSE_BUTTON_LEFT)) {
		rect_t changed_region = rect_from_2_points(_change_start, _change_end);
		
		changed_region.w++;
		changed_region.h++;
		
		// TODO: fix bug where the whole area is properly commited to the undo stack
		_push_to_undo(computer, changed_region);

		gfx_draw_ellipse(SPR_SURF(computer->ram->spritesheet.data), changed_region, _selected_color);
	}
}

static void _tool_ellipsef(computer_t *computer, point_t spritesheet_coord_under_mouse) {
	
}

static void _tool_bucket(computer_t *computer, point_t spritesheet_coord_under_mouse, point_t mouse_pos) {
	if (input_mouse_button_pressed(MOUSE_BUTTON_LEFT)) {
		rect_t limit = point_in_rect(mouse_pos, _layout.sprite_editor_focus_rect) ? get_in_frame_rect() : RECT(0, 0, SPRITESHEET_WIDTH, SPRITESHEET_HEIGHT);
		
		_push_to_undo(computer, limit);
		
		gfx_flood_fill(SPR_SURF(computer->ram->spritesheet.data), spritesheet_coord_under_mouse, _selected_color, limit);
	}
}

void sprite_editor_update(computer_t *computer) {
	point_t mouse_pos = input_get_mouse_pos();
	rect_t in_frame_rect = get_in_frame_rect();
	
	sprite_selector_update(computer, SNAP_MODE_ZOOM, _layout.sprite_selector_pos);

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

	// Swap primary and secondary selected color
	if (input_key_pressed(KEY_X)) {
		color_t temp = _selected_color;
		_selected_color = _secondary_selected_color;
		_secondary_selected_color = temp;
	}

	// Zoom for sprite selector
	if (input_key_pressed(KEY_MINUS) || input_mouse_scrolled(SCROLL_DIR_UP)) {
		sprite_selector_zoom_in();
	}

	if (input_key_pressed(KEY_EQUALS) || input_mouse_scrolled(SCROLL_DIR_DOWN)) {
		sprite_selector_zoom_out();
	}

	if (point_in_rect(mouse_pos, _layout.sprite_editor_full_rect)) {
		point_t spritesheet_coord_under_mouse = cam_screen_to_world(&_camera, mouse_pos);

		if (input_mouse_button_pressed(MOUSE_BUTTON_LEFT)) {
			_change_start = spritesheet_coord_under_mouse;
		}

		// Eyedropper
		// TODO: make seperate tool? with button and stuff and then switch to it while alt is held
		if ((input_key_held(KEY_LALT) || input_key_held(KEY_RALT))) {
			if (input_mouse_button_held(MOUSE_BUTTON_LEFT)) {
				_selected_color = computer->ram->spritesheet.data[spritesheet_coord_under_mouse.y * SPRITESHEET_WIDTH + spritesheet_coord_under_mouse.x];
			}
			return;
		}

		// TODO: Maybe remove the overlay and dynamic tracking of changes and just render previews to framebuffer and copy the currently editing rect region to a change object
		switch (_selected_tool) {
			// Behavior: if a selection exists apply any copy, paste cut actions to it
			// otherwise do it in the sprite selector
			// maybe just don't call the sprite selector as long as a selection exists
			
			// TODO: for selection copy it to another buffer seperate from the overlay
			// so part of it doesn't disappear when you move it outside of bounds

			// TODO: when selection active, you can only draw inside of the selection

			// TODO: seperate function for each tool
			case (TOOL_SELECT):
				_tool_select(computer, spritesheet_coord_under_mouse);
				break;
			case (TOOL_PENCIL):
				_tool_pencil(computer, spritesheet_coord_under_mouse);
				break;
			case (TOOL_LINE):
				_tool_line(computer, spritesheet_coord_under_mouse);
				break;
			case (TOOL_RECT):
				_tool_rect(computer, spritesheet_coord_under_mouse);
				break;
			case (TOOL_RECTF):
				_tool_rectf(computer, spritesheet_coord_under_mouse);
				break;
			case (TOOL_ELLIPSE):
				_tool_ellipse(computer, spritesheet_coord_under_mouse);
				break;
			case (TOOL_ELLIPSEF):
				_tool_ellipsef(computer, spritesheet_coord_under_mouse);
				break;
			case (TOOL_BUCKET):
				_tool_bucket(computer, spritesheet_coord_under_mouse, mouse_pos);
				break;
			default:
				break;
		}
	}
}

// TODO: probably move this to GUI
static void _draw_selection_rect(uint64_t ticks, surface_t surf, rect_t rect) {
	int thing = ticks % 30 < 15;
	for (int j = rect.x; j < rect.x+rect.w; j++) {
		color_t color = j % 3 == thing ? COLOR_BLACK : COLOR_WHITE;
		surf_set_pixel(surf, j, rect.y, color);
	}

	for (int j = rect.x; j < rect.x+rect.w; j++) {
		color_t color = j % 3 == thing ? COLOR_BLACK : COLOR_WHITE;
		surf_set_pixel(surf, j, rect.y+rect.h-1, color);
	}

	for (int i = rect.y; i < rect.y+rect.h; i++) {
		color_t color = i % 3 == thing ? COLOR_BLACK : COLOR_WHITE;
		surf_set_pixel(surf, rect.x, i, color);
	}

	for (int i = rect.y; i < rect.y+rect.h; i++) {
		color_t color = i % 3 == thing ? COLOR_BLACK : COLOR_WHITE;
		surf_set_pixel(surf, rect.x+rect.w - 1, i, color);
	}
}

// TODO: overlay is drawn over selected sprites, need to fix
void sprite_editor_draw(computer_t *computer) {
	framebuffer_t *fb = &computer->ram->framebuffer;
	surface_t fb_surf = FB_SURF(fb->data);
	rect_t in_frame_rect = get_in_frame_rect();
	rect_t in_frame_rect_in_sprites = get_in_frame_rect_in_sprites();
	
	int scale = _layout.sprite_editor_focus_rect.w / in_frame_rect.w;

	// TODO: also update sprite selector in frame rect when scrolling anywhere, just in the sprite editor

	// Draw spritesheet
	{
		_camera.pos = in_frame_rect.pos;
		_camera.zoom = scale;

		// Source rect is the entire spritesheet
		// TODO: make a global const or macro or something
		rect_t source_rect = RECT(0, 0, SPRITESHEET_WIDTH, SPRITESHEET_HEIGHT);

		rect_t dest_rect = {0};
		dest_rect.pos = cam_world_to_screen(&_camera, (point_t){0});

		dest_rect.w = source_rect.w * _camera.zoom;
		dest_rect.h = source_rect.h * _camera.zoom;

		gfx_draw_spritesheet_pro(computer->ram, source_rect, dest_rect, COLOR_NONE, _layout.sprite_editor_full_rect);

		gfx_draw_rect(fb_surf, RECT(_layout.sprite_editor_focus_rect.x - 1, _layout.sprite_editor_focus_rect.y - 1, _layout.sprite_editor_focus_rect.w + 2, _layout.sprite_editor_focus_rect.h + 2), COLOR_WHITE);
	}

	// Spritesheet / sprite selector
	sprite_selector_draw(computer, _layout.sprite_selector_pos, _layout.sprite_selector_buttons_start_pos);

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

	// Draw the overlay
	{
		rect_t source_rect = RECT(0, 0, SPRITESHEET_WIDTH, SPRITESHEET_HEIGHT);

		rect_t dest_rect = {0};
		dest_rect.pos = cam_world_to_screen(&_camera, (point_t){0});

		dest_rect.w = source_rect.w * _camera.zoom;
		dest_rect.h = source_rect.h * _camera.zoom;

		gfx_draw_surface_pro(fb, _overlay_surf, source_rect, dest_rect, COLOR_NONE, _layout.sprite_editor_full_rect);
	}
	
	// Draw the overlay on sprite selector as well
	gfx_draw_surface_rect(fb, _overlay_surf, _layout.sprite_selector_pos, get_page_rect(), COLOR_NONE);

	// Draw cursor
	{
		if (_selected_tool >= TOOL_PENCIL && _selected_tool <= TOOL_BUCKET) {
			if (point_in_rect(input_get_mouse_pos(), _layout.sprite_editor_full_rect)) {
				point_t spritesheet_coord_under_mouse = cam_screen_to_world(&_camera, input_get_mouse_pos());

				rect_t dest_rect = {0};
				dest_rect.pos = cam_world_to_screen(&_camera, spritesheet_coord_under_mouse);

				dest_rect.w = _camera.zoom;
				dest_rect.h = _camera.zoom;

				gfx_draw_filled_rect(fb_surf, dest_rect, _selected_color);
			}
		}
	}

	// Draw the selection outline
	if (!(_selection_start.x == _selection_end.x && _selection_start.y == _selection_end.y)) {
		rect_t selection = _get_selection();

		// TODO: use world_to_screen for this
		selection.x -= in_frame_rect.x;
		selection.y -= in_frame_rect.y;
	
		selection.x *= scale;
		selection.y *= scale;
	
		selection.w *= scale;
		selection.h *= scale;
		
		selection.x += _layout.sprite_editor_focus_rect.x;
		selection.y += _layout.sprite_editor_focus_rect.y;
		
		// gfx_draw_rect(fb_surf, selection, COLOR_LIGHTGRAY);
		_draw_selection_rect(computer->ram->ticks, fb_surf, selection);
	}

	// Selected color
	char buffer[32];
	gfx_draw_filled_rect(fb_surf, _layout.selected_color_rect, _selected_color);
	sprintf(buffer, "#%03d\n", _selected_color);
	gui_draw_text(computer->ram, GUI_FONT_INDEX, buffer, _layout.selected_color_label_pos, computer->ram->skin.font_color);

	// Secondary selected color
	gfx_draw_filled_rect(fb_surf, _layout.secondary_selected_color_rect, _secondary_selected_color);

	// gui_draw_text(computer->ram, 2, buffer, _layout.selected_color_label_pos, COLOR_NONE); // Testing not passing a color
	
	// Selected sprite preview
	gfx_draw_spritesheet_pro(computer->ram, in_frame_rect, _layout.selected_sprite_rect, COLOR_NONE, RECT(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT)); // TODO: fix so it adds the other rects to currently_editing_rect
	sprintf(buffer, "#%04d\n", get_sprite_index());
	gui_draw_text(computer->ram, GUI_FONT_INDEX, buffer, _layout.selected_sprite_label_pos, computer->ram->skin.font_color);

	// Sprite flags and color key
	sprite_t *selected_sprite = &computer->ram->sprites[get_sprite_index()];

	// Still using the macro because it is probably safer
	for (int i = 0; i < SPRITE_FLAGS_SIZE; i++) {
		uint32_t mask = 1U << i;

		bool old_val = (selected_sprite->flags & mask) != 0;

		point_t pos = button_array_get_pos(&skin_layout.sprite_flag_buttons, _layout.sprite_flags_start_pos, i);
		button_t button = button_array_get(&skin_layout.sprite_flag_buttons, i);
		
		bool new_val = gui_toggle_button(computer->ram, pos, button, old_val);
		if (new_val == old_val) {
			continue;
		}

		for (size_t y = 0; y < in_frame_rect_in_sprites.h; y++) {
			for (size_t x = 0; x < in_frame_rect_in_sprites.w; x++) {
				size_t sprite_index = sprite_coords_to_index(in_frame_rect_in_sprites.x + x, in_frame_rect_in_sprites.y + y);
				sprite_t *sprite = &computer->ram->sprites[sprite_index];
				
				if (new_val) {
					sprite->flags |= mask;
				} else {
					sprite->flags &= ~mask;
				}
			}
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

	// Debugging stuff, will keep for now
	// gfx_draw_rect(fb_surf, RECT(100, 100, 101, 21), COLOR_BLUE);
	// gfx_draw_ellipse(fb_surf, POINT(100, 100), POINT(201, 121), COLOR_RED);
	// gfx_draw_ellipse(fb_surf, POINT(201, 121), POINT(100, 100), COLOR_RED);
	// gfx_draw_rect(fb_surf, RECT(100, 100, 3, 3), COLOR_BLUE);
	// gfx_draw_ellipse(fb_surf, POINT(100, 100), 50, 200, COLOR_RED);
}
