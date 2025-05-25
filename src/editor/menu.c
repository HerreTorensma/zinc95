#include "menu.h"

#include <stdio.h>

#include "../backend/input.h"
#include "../backend/gui.h"
#include "../backend/gfx.h"

#include "code.h"
#include "sprite.h"
#include "map.h"
#include "sound.h"
#include "shared.h"

#define SAVE_ICON_INDEX 5858
#define PLAY_ICON_INDEX 5856

typedef enum workspace_type {
	WORKSPACE_CODE,
	WORKSPACE_SPRITE,
	WORKSPACE_MAP,
	WORKSPACE_SOUND,
} workspace_type_t;

static workspace_type_t _active_workspace = WORKSPACE_SPRITE;

typedef struct layout {
	point_t code_editor_button_pos;
	point_t sprite_editor_button_pos;
	point_t map_editor_button_pos;
	point_t sound_editor_button_pos;

	point_t save_button_pos;
	point_t play_button_pos;
} layout_t;

static const layout_t _layout = {
	.code_editor_button_pos = {192, 2},
	.sprite_editor_button_pos = {256, 2},
	.map_editor_button_pos = {320, 2},
	.sound_editor_button_pos = {384, 2},

	.save_button_pos = {606, 2},
	.play_button_pos = {622, 2},
};

void workspace_menu_init(computer_t *computer) {
	// Init the sprite selector (shared between sprite and map editor)
	sprite_selector_init(computer);

	code_editor_init(computer);
	sprite_editor_init(computer);
	map_editor_init(computer);
	sound_editor_init(computer);
}

void workspace_menu_update(computer_t *computer) {
	if (input_key_pressed(KEY_F1)) {
		_active_workspace = WORKSPACE_CODE;
	}
	if (input_key_pressed(KEY_F2)) {
		_active_workspace = WORKSPACE_SPRITE;
	}
	if (input_key_pressed(KEY_F3)) {
		_active_workspace = WORKSPACE_MAP;
	}
	if (input_key_pressed(KEY_F4)) {
		_active_workspace = WORKSPACE_SOUND;
	}
	if (input_key_pressed(KEY_F5)) {
		play_game(computer);
	}

	switch (_active_workspace) {
		case WORKSPACE_CODE:
			code_editor_update(computer);
			break;

		case WORKSPACE_SPRITE:
			sprite_editor_update(computer);
			break;

		case WORKSPACE_MAP:
			map_editor_update(computer);
			break;

		case WORKSPACE_SOUND:
			sound_editor_update(computer);
			break;

		default:
			break;
	}
}

void workspace_menu_draw(computer_t *computer) {
	gfx_clear(FB_SURF(computer->ram->framebuffer.data), 7);

	// gui_outset_frame(computer->ram, workspace_rect);

	switch (_active_workspace) {
		case WORKSPACE_CODE:
			code_editor_draw(computer);
			break;

		case WORKSPACE_SPRITE:
			sprite_editor_draw(computer);
			break;

		case WORKSPACE_MAP:
			map_editor_draw(computer);
			break;

		case WORKSPACE_SOUND:
			sound_editor_draw(computer);
			break;

		default:
			break;
	}

	// Menu bar
	// gui_outset_frame(computer->ram, bar_rect);
	gui_outset_frame(computer->ram, RECT(0, 0, SCREEN_WIDTH, 20));
	
	// gui_button(computer->ram, "", (rect_t){2, 2, 16, 16});
	
	if (gui_button_ex(computer->ram, "Code", RECT(_layout.code_editor_button_pos.x, _layout.code_editor_button_pos.y, 64, 16), _active_workspace == WORKSPACE_CODE)) {
		_active_workspace = WORKSPACE_CODE;
	}
	
	if (gui_button_ex(computer->ram, "Sprite", RECT(_layout.sprite_editor_button_pos.x, _layout.sprite_editor_button_pos.y, 64, 16), _active_workspace == WORKSPACE_SPRITE)) {
		_active_workspace = WORKSPACE_SPRITE;
	}
	
	if (gui_button_ex(computer->ram, "Map", RECT(_layout.map_editor_button_pos.x, _layout.map_editor_button_pos.y, 64, 16), _active_workspace == WORKSPACE_MAP)) {
		_active_workspace = WORKSPACE_MAP;
	}
	
	if (gui_button_ex(computer->ram, "Sound", RECT(_layout.sound_editor_button_pos.x, _layout.sound_editor_button_pos.y, 64, 16), _active_workspace == WORKSPACE_SOUND)) {
		_active_workspace = WORKSPACE_SOUND;
	}
	
	if (gui_press_button(computer->ram, "", RECT(_layout.save_button_pos.x, _layout.save_button_pos.y, 16, 16))) {
		game_save(computer, "game.zinc95");
	}
	// api_spr(computer, 5858, SCREEN_WIDTH-16-16-2, 2, 2, 2, 1);
	// api_spr(computer, 5858, SCREEN_WIDTH-16-16-2, 2, 2, 2);
	gfx_draw_sprites(computer->ram, SAVE_ICON_INDEX, POINT(SCREEN_WIDTH-16-16-2, 2), 2, 2);
	
	if (gui_press_button(computer->ram, "", RECT(_layout.play_button_pos.x, _layout.play_button_pos.y, 16, 16))) {
		play_game(computer);
	}
	// api_spr(computer, 5856, SCREEN_WIDTH-16-2, 2, 2, 2, 1);
	// api_spr(computer, 5856, SCREEN_WIDTH-16-2, 2, 2, 2);
	gfx_draw_sprites(computer->ram, PLAY_ICON_INDEX, POINT(SCREEN_WIDTH-16-2, 2), 2, 2);
}