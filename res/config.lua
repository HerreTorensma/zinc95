
return {
	skin = "default",
	
	palette = "default",
	
	code_editor = {
		tab_size = 4,
		scroll_speed = 3,
		
		selection_color = 11,
		cursor_color = 9,
		
		token_colors = {
			keyword = 12,
			builtin_function = 6,
			identifier = 0,
			literal = 5,
			string = 2,
			comment = 7,
			operator = 9,
			whitespace = 255,
		},
	},
	
	-- NOTE: setting the same keybind for multiple actions can cause unexpected behavior! Proceed with caution.
	keybinds = {
		global = {
			run = {"Ctrl", "R"},

			toggle_terminal = {"Esc"},
	
			switch_to_code_editor = {"F1"},
			switch_to_sprite_editor = {"F2"},
			switch_to_map_editor = {"F3"},
			switch_to_sound_editor = {"F4"},
			switch_to_music_editor = {"F5"},
	
			undo = {"Ctrl", "Z"},
			redo = {"Ctrl", "Shift", "Z"},
			save = {"Ctrl", "S"},

			copy = {"Ctrl", "C"},
			paste = {"Ctrl", "V"},
			cut = {"Ctrl", "X"},

			select_all = {"Ctrl", "A"},
	
			prev_tool = {"Shift", "N"},
			next_tool = {"N"},
		}
	
		code_editor = {
			prev_file = {"Ctrl", "Up"},
			next_file = {"Ctrl", "Down"},

			toggle_comment = {"Ctrl", "/"},
	
			move_line_or_selection_up = {"Alt", "Up"},
			move_line_or_selection_down = {"Alt", "Down"},

			decrease_indentation = {"Shift", "Tab"},
			increase_indentation = {"Tab"},

			jump_to_prev_word = {"Ctrl", "Left"},
			jump_to_next_word = {"Ctrl", "Right"},
		},
	
		sprite_editor = {
			switch_primary_and_secondary_color = {"X"},

			select_tool = {"M"},
			brush_tool = {"B"},
			line_tool = {"L"},
			rect_tool = {"R"},
			ellipse_tool = {"C"},
			bucket_tool = {"G"},

			move_frame_left = {"A"},
			move_frame_right = {"D"},
			move_frame_up = {"W"},
			move_frame_down = {"S"},
		},
	
		map_editor = {
			pan_modifier = {"Space"}

			zoom_out = {"Ctrl", "-"},
			zoom_in = {"Ctrl", "+"},
		},
	
		sound_editor = {
			play_active_sound = "Space",
		},

		music_editor = {
			play_song = "Space",
		},
	},
}
