#include "menu.h"

#include <stdio.h>

#include "../util/util.h"
#include "../api/api.h"
#include "../backend/input.h"

#include "code.h"
#include "sprite.h"
#include "map.h"
#include "sound.h"

typedef enum workspace_type {
	WORKSPACE_CODE,
	WORKSPACE_SPRITE,
	WORKSPACE_MAP,
	WORKSPACE_SOUND,
} workspace_type_t;

static workspace_type_t active_workspace = WORKSPACE_SPRITE;

static rect_t bar_rect = {0};
static rect_t screen_rect = {0};

rect_t workspace_rect = {0, 20, SCREEN_WIDTH, SCREEN_HEIGHT - 20};

void workspace_menu_init(computer_t *computer) {
	bar_rect = (rect_t){0, 0, SCREEN_WIDTH, 20};
	screen_rect = (rect_t){0, 0, SCREEN_WIDTH, SCREEN_HEIGHT};

	// console_init(computer);
	code_editor_init(computer);
	sprite_editor_init(computer);
	map_editor_init(computer);
	sound_editor_init(computer);
}

void workspace_menu_update(computer_t *computer) {
	if (api_keyp(computer, KEY_F1)) {
		active_workspace = WORKSPACE_CODE;
	}
	if (api_keyp(computer, KEY_F2)) {
		active_workspace = WORKSPACE_SPRITE;
	}
	if (api_keyp(computer, KEY_F3)) {
		active_workspace = WORKSPACE_MAP;
	}
	if (api_keyp(computer, KEY_F4)) {
		active_workspace = WORKSPACE_SOUND;
	}
	if (api_keyp(computer, KEY_F5)) {
		play_game(computer);
	}

	switch (active_workspace) {
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
	api_cls(computer, 7);

	draw_out_frame(computer, screen_rect);
	
	// Menu bar
	draw_out_frame(computer, bar_rect);
	
	button(computer, "", (rect_t){2, 2, 16, 16});
	
	if (button_ex(computer, "Code", (rect_t){128 + 64*1, 2, 64, 16}, active_workspace == WORKSPACE_CODE)) {
		active_workspace = WORKSPACE_CODE;
	}
	
	if (button_ex(computer, "Sprite", (rect_t){128 + 64*2, 2, 64, 16}, active_workspace == WORKSPACE_SPRITE)) {
		active_workspace = WORKSPACE_SPRITE;
	}
	
	if (button_ex(computer, "Map", (rect_t){128 + 64*3, 2, 64, 16}, active_workspace == WORKSPACE_MAP)) {
		active_workspace = WORKSPACE_MAP;
	}
	
	if (button_ex(computer, "Sound", (rect_t){128 + 64*4, 2, 64, 16}, active_workspace == WORKSPACE_SOUND)) {
		active_workspace = WORKSPACE_SOUND;
	}
	
	if (press_button(computer, "", (rect_t){SCREEN_WIDTH-16-16-2, 2, 16, 16})) {
		game_save(computer, "game.zinc95");
	}
	api_spr(computer, 5858, SCREEN_WIDTH-16-16-2, 2, 2, 2, 1);
	
	if (press_button(computer, "", (rect_t){SCREEN_WIDTH-16-2, 2, 16, 16})) {
		play_game(computer);
	}
	api_spr(computer, 5856, SCREEN_WIDTH-16-2, 2, 2, 2, 1);

	switch (active_workspace) {
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
}