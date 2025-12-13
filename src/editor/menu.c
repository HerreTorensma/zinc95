#include "menu.h"

#include "../backend/input.h"
#include "../backend/gui.h"
#include "../backend/gfx.h"
#include "../res.h"

#include "code.h"
#include "sprite.h"
#include "map.h"
#include "sound.h"
#include "music.h"
#include "shared.h"

#define SAVE_ICON_INDEX 5858
#define PLAY_ICON_INDEX 5856

typedef enum workspace_type {
	WORKSPACE_CODE,
	WORKSPACE_SPRITE,
	WORKSPACE_MAP,
	WORKSPACE_SOUND,
	WORKSPACE_MUSIC,
} workspace_type_t;

static workspace_type_t _active_workspace = WORKSPACE_SPRITE;

static struct {
	point_t zinc_button_pos;
	point_t code_editor_button_pos;
	point_t sprite_editor_button_pos;
	point_t map_editor_button_pos;
	point_t sound_editor_button_pos;
	point_t music_editor_button_pos;

	point_t save_button_pos;
	point_t play_button_pos;
}
_layout = {
	.zinc_button_pos = {2, 2},
	.code_editor_button_pos = {160, 2},
	.sprite_editor_button_pos = {224, 2},
	.map_editor_button_pos = {288, 2},
	.sound_editor_button_pos = {352, 2},
	.music_editor_button_pos = {416, 2},

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
	// --- Global keybinds ---
	if (is_keybind_pressed(g_keybinds.global.run)) {
		if (computer->game_running) {
			quit_game(computer);
			play_game(computer);
		} else {
			play_game(computer);
		}

		return;
	}

	// TODO: I should probably check if computer.game_path exists
	// yes I definitely should bc the program crashes if you save a game without a name

	// Maybe it already fills it automatically?
	// and only check if inside the editor
	// But maybe for this and the run one above I'll move it into shared.h/c and call it from every editor seperately for more control
	if (is_keybind_pressed(g_keybinds.global.save)) {
		game_save(computer, computer->game_path);
		return;
	}

	// TODO: I think that if you have one keybind that's Ctrl + R and one thats R
	// then Ctrl + R should take priority and not execute the R
	// like filter candidates based on amount of modifiers or something
	// but thats a bit complicated for now, I'll look into it later
	if (is_keybind_pressed(g_keybinds.global.switch_to_code_editor)) {
		_active_workspace = WORKSPACE_CODE;
		return;
	}
	if (is_keybind_pressed(g_keybinds.global.switch_to_sprite_editor)) {
		_active_workspace = WORKSPACE_SPRITE;
		return;
	}
	if (is_keybind_pressed(g_keybinds.global.switch_to_map_editor)) {
		_active_workspace = WORKSPACE_MAP;
		return;
	}
	if (is_keybind_pressed(g_keybinds.global.switch_to_sound_editor)) {
		_active_workspace = WORKSPACE_SOUND;
		return;
	}
	if (is_keybind_pressed(g_keybinds.global.switch_to_music_editor)) {
		_active_workspace = WORKSPACE_MUSIC;
		return;
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
	gfx_clear(FB_SURF(computer->ram->framebuffer.data), 0);

	// Draw active editor part of skin
	gfx_draw_surface_rect(&computer->ram->framebuffer, SKIN_SURF(computer->ram->skin.data), POINT(0, 0), RECT(SCREEN_WIDTH * _active_workspace, 0, SCREEN_WIDTH, SCREEN_HEIGHT), COLOR_NONE);

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

		case WORKSPACE_MUSIC:
			music_editor_draw(computer);
			break;

		default:
			break;
	}

	if (gui_button(
		computer->ram,
		_layout.zinc_button_pos,
		g_skin_layout.zinc_button,
		false
	)) {
		
	}

	// Editors
	if (gui_button(computer->ram, _layout.code_editor_button_pos, g_skin_layout.code_button, _active_workspace == WORKSPACE_CODE)) {
		_active_workspace = WORKSPACE_CODE;
	}
	
	if (gui_button(computer->ram, _layout.sprite_editor_button_pos, g_skin_layout.sprite_button, _active_workspace == WORKSPACE_SPRITE)) {
		_active_workspace = WORKSPACE_SPRITE;
	}
	
	if (gui_button(computer->ram, _layout.map_editor_button_pos, g_skin_layout.map_button, _active_workspace == WORKSPACE_MAP)) {
		_active_workspace = WORKSPACE_MAP;
	}
	
	if (gui_button(computer->ram, _layout.sound_editor_button_pos, g_skin_layout.sound_button, _active_workspace == WORKSPACE_SOUND)) {
		_active_workspace = WORKSPACE_SOUND;
	}

	if (gui_button(computer->ram, _layout.music_editor_button_pos, g_skin_layout.music_button, _active_workspace == WORKSPACE_MUSIC)) {
		_active_workspace = WORKSPACE_MUSIC;
	}

	// Save and load
	if (gui_press_button(computer->ram, _layout.save_button_pos, g_skin_layout.save_button)) {
		game_save(computer, computer->game_path);
	}
	
	if (!computer->game_running) {
		if (gui_press_button(computer->ram, _layout.play_button_pos, g_skin_layout.play_button)) {
			play_game(computer);
		}
	} else {
		if (gui_press_button(computer->ram, _layout.play_button_pos, g_skin_layout.stop_button)) {
			quit_game(computer);
		}
	}
}
