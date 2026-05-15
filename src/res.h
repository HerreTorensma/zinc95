/*
Builtin resources, right in the source code so they won't have to be read at runtime
*/

#pragma once

#include "core/input.h"
#include "computer.h"

static const palette_t g_builtin_palette = (palette_t){
	.colors = {
		{0, 0, 0},
		{0, 0, 170},
		{0, 170, 0},
		{0, 170, 170},
		{170, 0, 0},
		{170, 0, 170},
		{170, 85, 0},
		{170, 170, 170},
		{85, 85, 85},
		{85, 85, 255},
		{85, 255, 85},
		{85, 255, 255},
		{255, 85, 85},
		{255, 85, 255},
		{255, 255, 85},
		{255, 255, 255},
		{0, 0, 0},
		{16, 16, 16},
		{32, 32, 32},
		{53, 53, 53},
		{69, 69, 69},
		{85, 85, 85},
		{101, 101, 101},
		{117, 117, 117},
		{138, 138, 138},
		{154, 154, 154},
		{170, 170, 170},
		{186, 186, 186},
		{202, 202, 202},
		{223, 223, 223},
		{239, 239, 239},
		{255, 255, 255},
		{0, 0, 255},
		{65, 0, 255},
		{130, 0, 255},
		{190, 0, 255},
		{255, 0, 255},
		{255, 0, 190},
		{255, 0, 130},
		{255, 0, 65},
		{255, 0, 0},
		{255, 65, 0},
		{255, 130, 0},
		{255, 190, 0},
		{255, 255, 0},
		{190, 255, 0},
		{130, 255, 0},
		{65, 255, 0},
		{0, 255, 0},
		{0, 255, 65},
		{0, 255, 130},
		{0, 255, 190},
		{0, 255, 255},
		{0, 190, 255},
		{0, 130, 255},
		{0, 65, 255},
		{130, 130, 255},
		{158, 130, 255},
		{190, 130, 255},
		{223, 130, 255},
		{255, 130, 255},
		{255, 130, 223},
		{255, 130, 190},
		{255, 130, 158},
		{255, 130, 130},
		{255, 158, 130},
		{255, 190, 130},
		{255, 223, 130},
		{255, 255, 130},
		{223, 255, 130},
		{190, 255, 130},
		{158, 255, 130},
		{130, 255, 130},
		{130, 255, 158},
		{130, 255, 190},
		{130, 255, 223},
		{130, 255, 255},
		{130, 223, 255},
		{130, 190, 255},
		{130, 158, 255},
		{186, 186, 255},
		{202, 186, 255},
		{223, 186, 255},
		{239, 186, 255},
		{255, 186, 255},
		{255, 186, 239},
		{255, 186, 223},
		{255, 186, 202},
		{255, 186, 186},
		{255, 202, 186},
		{255, 223, 186},
		{255, 239, 186},
		{255, 255, 186},
		{239, 255, 186},
		{223, 255, 186},
		{202, 255, 186},
		{186, 255, 186},
		{186, 255, 202},
		{186, 255, 223},
		{186, 255, 239},
		{186, 255, 255},
		{186, 239, 255},
		{186, 223, 255},
		{186, 202, 255},
		{0, 0, 113},
		{28, 0, 113},
		{57, 0, 113},
		{85, 0, 113},
		{113, 0, 113},
		{113, 0, 85},
		{113, 0, 57},
		{113, 0, 28},
		{113, 0, 0},
		{113, 28, 0},
		{113, 57, 0},
		{113, 85, 0},
		{113, 113, 0},
		{85, 113, 0},
		{57, 113, 0},
		{28, 113, 0},
		{0, 113, 0},
		{0, 113, 28},
		{0, 113, 57},
		{0, 113, 85},
		{0, 113, 113},
		{0, 85, 113},
		{0, 57, 113},
		{0, 28, 113},
		{57, 57, 113},
		{69, 57, 113},
		{85, 57, 113},
		{97, 57, 113},
		{113, 57, 113},
		{113, 57, 97},
		{113, 57, 85},
		{113, 57, 69},
		{113, 57, 57},
		{113, 69, 57},
		{113, 85, 57},
		{113, 97, 57},
		{113, 113, 57},
		{97, 113, 57},
		{85, 113, 57},
		{69, 113, 57},
		{57, 113, 57},
		{57, 113, 69},
		{57, 113, 85},
		{57, 113, 97},
		{57, 113, 113},
		{57, 97, 113},
		{57, 85, 113},
		{57, 69, 113},
		{81, 81, 113},
		{89, 81, 113},
		{97, 81, 113},
		{105, 81, 113},
		{113, 81, 113},
		{113, 81, 105},
		{113, 81, 97},
		{113, 81, 89},
		{113, 81, 81},
		{113, 89, 81},
		{113, 97, 81},
		{113, 105, 81},
		{113, 113, 81},
		{105, 113, 81},
		{97, 113, 81},
		{89, 113, 81},
		{81, 113, 81},
		{81, 113, 89},
		{81, 113, 97},
		{81, 113, 105},
		{81, 113, 113},
		{81, 105, 113},
		{81, 97, 113},
		{81, 89, 113},
		{0, 0, 65},
		{16, 0, 65},
		{32, 0, 65},
		{49, 0, 65},
		{65, 0, 65},
		{65, 0, 49},
		{65, 0, 32},
		{65, 0, 16},
		{65, 0, 0},
		{65, 16, 0},
		{65, 32, 0},
		{65, 49, 0},
		{65, 65, 0},
		{49, 65, 0},
		{32, 65, 0},
		{16, 65, 0},
		{0, 65, 0},
		{0, 65, 16},
		{0, 65, 32},
		{0, 65, 49},
		{0, 65, 65},
		{0, 49, 65},
		{0, 32, 65},
		{0, 16, 65},
		{32, 32, 65},
		{40, 32, 65},
		{49, 32, 65},
		{57, 32, 65},
		{65, 32, 65},
		{65, 32, 57},
		{65, 32, 49},
		{65, 32, 40},
		{65, 32, 32},
		{65, 40, 32},
		{65, 49, 32},
		{65, 57, 32},
		{65, 65, 32},
		{57, 65, 32},
		{49, 65, 32},
		{40, 65, 32},
		{32, 65, 32},
		{32, 65, 40},
		{32, 65, 49},
		{32, 65, 57},
		{32, 65, 65},
		{32, 57, 65},
		{32, 49, 65},
		{32, 40, 65},
		{45, 45, 65},
		{49, 45, 65},
		{53, 45, 65},
		{61, 45, 65},
		{65, 45, 65},
		{65, 45, 61},
		{65, 45, 53},
		{65, 45, 49},
		{65, 45, 45},
		{65, 49, 45},
		{65, 53, 45},
		{65, 61, 45},
		{65, 65, 45},
		{61, 65, 45},
		{53, 65, 45},
		{49, 65, 45},
		{45, 65, 45},
		{45, 65, 49},
		{45, 65, 53},
		{45, 65, 61},
		{45, 65, 65},
		{45, 61, 65},
		{45, 53, 65},
		{45, 49, 65},
		{0, 0, 0},
		{0, 0, 0},
		{0, 0, 0},
		{0, 0, 0},
		{0, 0, 0},
		{0, 0, 0},
		{0, 0, 0},
		{0, 0, 0},
	},
};

// TODO: make substructs for each editor like in g_keybinds
static struct {

	struct {
		button_t zinc_button;
	
		button_t code_button;
		button_t sprite_button;
		button_t map_button;
		button_t sound_button;
		button_t music_button;
	
		button_t save_button;
		button_t play_button;
		button_t stop_button;

		rect_t gui_font_rect;
		rect_t code_editor_font_rect;

		// Shared between sprite and map editor
		// TODO: maybe make a seperate one for each
		button_matrix_t spritesheet_areas_button_matrix;
		button_array_t spritesheet_snap_mode_buttons;
	} global;

	struct {
		button_t file_button;
		button_t add_file_button;
	} code_editor;

	struct {
		button_array_t tool_buttons;

		button_matrix_t sprite_flags_button_matrix;
		button_t color_key_button;
	} sprite_editor;

	struct {
		button_array_t entity_tool_buttons;

		button_t toggle_layer_button;
		
		button_t entity_layer_button;
		button_array_t tile_layer_buttons;
	} map_editor;

	struct {
		button_matrix_t patterns_button_matrix;

		button_t instrument_button;

		button_t sine_wave_button;
		button_t square_wave_button;
		button_t triangle_wave_button;
		button_t sawtooth_wave_button;
		button_t noise_wave_button;
	} sound_editor;

	struct {
		button_matrix_t arrangements_button_matrix;

		button_t edit_pattern_button;
	
		button_t pattern_inc_button;
		button_t pattern_dec_button;
	} music_editor;
}
g_skin_layout = {
	.global = {
		.zinc_button = {
			.unpressed_rect = {{3568, 44, 16, 16}},
			.pressed_rect = {{3568, 60, 16, 16}},
		},
	
		.code_button = {
			.unpressed_rect = {{3200, 44, 64, 16}},
			.pressed_rect = {{3200, 60, 64, 16}},
		},
	
		.sprite_button = {
			.unpressed_rect = {{3264, 44, 64, 16}},
			.pressed_rect = {{3264, 60, 64, 16}},
		},
	
		.map_button = {
			.unpressed_rect = {{3328, 44, 64, 16}},
			.pressed_rect = {{3328, 60, 64, 16}},
		},
	
		.sound_button = {
			.unpressed_rect = {{3392, 44, 64, 16}},
			.pressed_rect = {{3392, 60, 64, 16}},
		},
	
		.music_button = {
			.unpressed_rect = {{3456, 44, 64, 16}},
			.pressed_rect = {{3456, 60, 64, 16}},
		},
	
		.save_button = {
			.unpressed_rect = {{3520, 44, 16, 16}},
			.pressed_rect = {{3520, 60, 16, 16}},
		},
	
		.play_button = {
			.unpressed_rect = {{3536, 44, 16, 16}},
			.pressed_rect = {{3536, 60, 16, 16}},
		},
	
		.stop_button = {
			.unpressed_rect = {{3552, 44, 16, 16}},
			.pressed_rect = {{3552, 60, 16, 16}},
		},

		.gui_font_rect = {{3200, 432, 384, 32}},
		.code_editor_font_rect = {{3200, 464, 384, 16}},

		.spritesheet_areas_button_matrix = {
			.base = {
				.unpressed_rect = {{3368, 152, 28, 14}},
				.pressed_rect = {{3480, 152, 28, 14}},
			},
			.rows = 8,
			.columns = 4,
			.row_increase = 14,
			.column_increase = 28,
			.v_break = -1,
			.h_break = -1,
		},

		.spritesheet_snap_mode_buttons = {
			.base = {
				.unpressed_rect = {{3368, 264, 14, 14}},
				.pressed_rect = {{3368, 278, 14, 14}},
			},
			.increase = {14, 0},
			.amount = 2,
		},
	},

	.code_editor = {
		.file_button = {
			.unpressed_rect = {{3200, 316, 64, 13}},
			.pressed_rect = {{3264, 316, 64, 13}},
		},

		.add_file_button = {
			.unpressed_rect = {{3200, 329, 13, 13}},
			.pressed_rect = {{3213, 329, 13, 13}},
		},
	},

	.sprite_editor = {
		.tool_buttons = {
			.base = {
				.unpressed_rect = {{3296, 76, 16, 16}},
				.pressed_rect = {{3312, 76, 16, 16}},
			},
			.increase = {0, 16},
			.amount = 9,
		},

		.sprite_flags_button_matrix = {
			.base = {
				.unpressed_rect = {{3600, 156, 12, 12}},
				.pressed_rect = {{3600, 228, 12, 12}},
			},

			.rows = 6,
			.columns = 4,
			.row_increase = 12,
			.column_increase = 12,
			.v_break = -1,
			.h_break = -1,
		},
		
		.color_key_button = {
			.unpressed_rect = {{3584, 20, 12, 12}},
			.pressed_rect = {{3584, 32, 12, 12}},
		},
	},

	.map_editor = {
		.entity_tool_buttons = {
			.base = {
				.unpressed_rect = {{3328, 76, 16, 16}},
				.pressed_rect = {{3344, 76, 16, 16}},
			},
			.increase = {0, 16},
			.amount = 2,
		},

		.toggle_layer_button = {
			.unpressed_rect = {{3296, 236, 16, 16}},
			.pressed_rect = {{3312, 236, 16, 16}},
		},
		
		.entity_layer_button = {
			.unpressed_rect = {{3200, 236, 48, 16}},
			.pressed_rect = {{3248, 236, 48, 16}},
		},
	
		.tile_layer_buttons = {
			.base = {
				.unpressed_rect = {{3200, 252, 48, 16}},
				.pressed_rect = {{3248, 252, 48, 16}},
			},
			.increase = {0, 16},
			.amount = 4,
		},
	},

	.sound_editor = {
		.patterns_button_matrix = {
			.base = {
				.unpressed_rect = {{3264, 366, 8, 8}},
				.pressed_rect = {{3392, 366, 8, 8}},
			},
			.rows = 7 * 4,
			.columns = 14,
			.row_increase = 8,
			.column_increase = 8,
	
			.v_break_size = 6,
			.v_break = 7,
	
			.h_break_size = 0,
			.h_break = -1,
		},
		
		.instrument_button = {
			.unpressed_rect = {{3384, 96, 64, 12}},
			.pressed_rect = {{3448, 96, 64, 12}},
		},

		.sine_wave_button = {
			.unpressed_rect = {{3200, 350, 32, 16}},
			.pressed_rect = {{3232, 350, 32, 16}},
		},
		.square_wave_button = {
			.unpressed_rect = {{3200, 366, 32, 16}},
			.pressed_rect = {{3232, 366, 32, 16}},
		},
		.triangle_wave_button = {
			.unpressed_rect = {{3200, 382, 32, 16}},
			.pressed_rect = {{3232, 382, 32, 16}},
		},
		.sawtooth_wave_button = {
			.unpressed_rect = {{3200, 398, 32, 16}},
			.pressed_rect = {{3232, 398, 32, 16}},
		},
		.noise_wave_button = {
			.unpressed_rect = {{3200, 414, 32, 16}},
			.pressed_rect = {{3232, 414, 32, 16}},
		},
	},

	.music_editor = {
		.arrangements_button_matrix = {
			.base = { // TODO: make different from sfx_picker_buttons
				.unpressed_rect = {{3264, 366, 8, 8}},
				.pressed_rect = {{3392, 366, 8, 8}},
			},
			.rows = 2,
			.columns = 8 * 8,
			.row_increase = 8,
			.column_increase = 8,

			.v_break_size = 0,
			.v_break = -1,
			
			.h_break_size = 8,
			.h_break = 8,
		},

		.edit_pattern_button = {
			.unpressed_rect = {{3492, 328, 54, 14}},
			.pressed_rect = {{3492, 342, 54, 14}},
		},

		.pattern_inc_button = {
			.unpressed_rect = {{3546, 328, 14, 7}},
			.pressed_rect = {{3546, 342, 14, 7}},
		},

		.pattern_dec_button = {
			.unpressed_rect = {{3546, 335, 14, 7}},
			.pressed_rect = {{3546, 349, 14, 7}},
		},
	},
};

// Default keybinds
static struct {
	struct {
		keybind_t run;

		keybind_t toggle_terminal;

		keybind_t switch_to_code_editor;
		keybind_t switch_to_sprite_editor;
		keybind_t switch_to_map_editor;
		keybind_t switch_to_sound_editor;
		keybind_t switch_to_music_editor;

		keybind_t undo;
		keybind_t redo;
		keybind_t save;

		keybind_t copy;
		keybind_t paste;
		keybind_t cut;
	
		keybind_t select_all;
		keybind_t deselect;
		keybind_t delete_selection;

		keybind_t active_sprite_move_left;
		keybind_t active_sprite_move_right;
		keybind_t active_sprite_move_up;
		keybind_t active_sprite_move_down;
	} global;

	struct {
		keybind_t switch_primary_and_secondary_color;

		keybind_t select_tool;
		keybind_t brush_tool;
		keybind_t line_tool;
		keybind_t rect_tool;
		keybind_t ellipse_tool;
		keybind_t bucket_tool;
	} sprite_editor;
}
g_keybinds = {
	.global = {
		.run = {MODIFIER_CTRL, KEY_R},

		.toggle_terminal = {MODIFIER_NONE, KEY_ESC},

		.switch_to_code_editor = {MODIFIER_NONE, KEY_F1},
		.switch_to_sprite_editor = {MODIFIER_NONE, KEY_F2},
		.switch_to_map_editor = {MODIFIER_NONE, KEY_F3},
		.switch_to_sound_editor = {MODIFIER_NONE, KEY_F4},
		.switch_to_music_editor = {MODIFIER_NONE, KEY_F5},

		.undo = {MODIFIER_CTRL, KEY_Z},
		.redo = {MODIFIER_CTRL | MODIFIER_SHIFT, KEY_Z},
		.save = {MODIFIER_CTRL, KEY_S},

		.copy = {MODIFIER_CTRL, KEY_C},
		.paste = {MODIFIER_CTRL, KEY_V},
		.cut = {MODIFIER_CTRL, KEY_X},
	
		.select_all = {MODIFIER_CTRL, KEY_A},
		.deselect = {MODIFIER_CTRL, KEY_D},
		.delete_selection = {MODIFIER_NONE, KEY_DELETE},

		.active_sprite_move_left = {MODIFIER_NONE, KEY_A},
		.active_sprite_move_right = {MODIFIER_NONE, KEY_D},
		.active_sprite_move_up = {MODIFIER_NONE, KEY_W},
		.active_sprite_move_down = {MODIFIER_NONE, KEY_S},
	},

	.sprite_editor = {
		.switch_primary_and_secondary_color = {MODIFIER_NONE, KEY_X},
		// .eyedropper_modifier = MODIFIER_ALT,

		.select_tool = {MODIFIER_NONE, KEY_M},
		.brush_tool = {MODIFIER_NONE, KEY_B},
		.line_tool = {MODIFIER_NONE, KEY_L},
		.rect_tool = {MODIFIER_NONE, KEY_R},
		.ellipse_tool = {MODIFIER_NONE, KEY_C},
		.bucket_tool = {MODIFIER_NONE, KEY_G},
	},
};
