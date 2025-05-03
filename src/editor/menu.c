#include "menu.h"

#include <stdio.h>

// #include "../api/api.h"
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

#define MENU_BUTTONS_OFFSET 192
// #define MENU_BUTTONS_OFFSET GUI_BORDER_WIDTH
#define BUTTON_WIDTH GUI_STANDARD_BUTTON_SIZE * 4
#define BUTTON_HEIGHT GUI_STANDARD_BUTTON_SIZE

typedef enum workspace_type {
	WORKSPACE_CODE,
	WORKSPACE_SPRITE,
	WORKSPACE_MAP,
	WORKSPACE_SOUND,
} workspace_type_t;

static workspace_type_t active_workspace = WORKSPACE_SPRITE;

static rect_t bar_rect = {0};
static rect_t screen_rect = {0};

// rect_t workspace_rect = {0, 20, SCREEN_WIDTH, SCREEN_HEIGHT - 20};
rect_t workspace_rect = {0};

void workspace_menu_init(computer_t *computer) {
	bar_rect = (rect_t){0, 0, SCREEN_WIDTH, 20};
	// screen_rect = (rect_t){0, 0, SCREEN_WIDTH, SCREEN_HEIGHT};
	workspace_rect = rect_put_below(bar_rect, RECT(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT - bar_rect.h), 0);

	// Init the sprite selector
	sprite_selector_init(computer);

	// console_init(computer);
	code_editor_init(computer);
	sprite_editor_init(computer);
	map_editor_init(computer);
	sound_editor_init(computer);
}

void workspace_menu_update(computer_t *computer) {
	if (input_key_pressed(KEY_F1)) {
		active_workspace = WORKSPACE_CODE;
	}
	if (input_key_pressed(KEY_F2)) {
		active_workspace = WORKSPACE_SPRITE;
	}
	if (input_key_pressed(KEY_F3)) {
		active_workspace = WORKSPACE_MAP;
	}
	if (input_key_pressed(KEY_F4)) {
		active_workspace = WORKSPACE_SOUND;
	}
	if (input_key_pressed(KEY_F5)) {
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
	gfx_clear(&computer->ram->framebuffer, 7);

	gui_outset_frame(computer->ram, workspace_rect);

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

	// Menu bar
	gui_outset_frame(computer->ram, bar_rect);
	
	// gui_button(computer->ram, "", (rect_t){2, 2, 16, 16});
	
	if (gui_button_ex(computer->ram, "Code", (rect_t){MENU_BUTTONS_OFFSET + BUTTON_WIDTH*0, GUI_BORDER_WIDTH, BUTTON_WIDTH, BUTTON_HEIGHT}, active_workspace == WORKSPACE_CODE)) {
		active_workspace = WORKSPACE_CODE;
	}
	
	if (gui_button_ex(computer->ram, "Sprite", (rect_t){MENU_BUTTONS_OFFSET + BUTTON_WIDTH*1, GUI_BORDER_WIDTH, BUTTON_WIDTH, BUTTON_HEIGHT}, active_workspace == WORKSPACE_SPRITE)) {
		active_workspace = WORKSPACE_SPRITE;
	}
	
	if (gui_button_ex(computer->ram, "Map", (rect_t){MENU_BUTTONS_OFFSET + BUTTON_WIDTH*2, GUI_BORDER_WIDTH, BUTTON_WIDTH, BUTTON_HEIGHT}, active_workspace == WORKSPACE_MAP)) {
		active_workspace = WORKSPACE_MAP;
	}
	
	if (gui_button_ex(computer->ram, "Sound", (rect_t){MENU_BUTTONS_OFFSET + BUTTON_WIDTH*3, GUI_BORDER_WIDTH, BUTTON_WIDTH, BUTTON_HEIGHT}, active_workspace == WORKSPACE_SOUND)) {
		active_workspace = WORKSPACE_SOUND;
	}
	
	if (gui_press_button(computer->ram, "", (rect_t){SCREEN_WIDTH-16-16-2, 2, 16, 16})) {
		game_save(computer, "game.zinc95");
	}
	// api_spr(computer, 5858, SCREEN_WIDTH-16-16-2, 2, 2, 2, 1);
	// api_spr(computer, 5858, SCREEN_WIDTH-16-16-2, 2, 2, 2);
	gfx_draw_sprites(computer->ram, SAVE_ICON_INDEX, POINT(SCREEN_WIDTH-16-16-2, 2), 2, 2);
	
	if (gui_press_button(computer->ram, "", (rect_t){SCREEN_WIDTH-16-2, 2, 16, 16})) {
		play_game(computer);
	}
	// api_spr(computer, 5856, SCREEN_WIDTH-16-2, 2, 2, 2, 1);
	// api_spr(computer, 5856, SCREEN_WIDTH-16-2, 2, 2, 2);
	gfx_draw_sprites(computer->ram, PLAY_ICON_INDEX, POINT(SCREEN_WIDTH-16-2, 2), 2, 2);
}