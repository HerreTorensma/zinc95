#include "txt.h"

#include <string.h>
#include <stdio.h>
#include <sys/types.h>

#include "input.h"
#include "../common/io.h"

// Directly generates the rgb framebuffer from the textbuffer
void txt_generate_rgb_framebuffer(computer_t *computer) {
	for (int y = 0; y < TEXTBUFFER_HEIGHT; y++) {
		for (int x = 0; x < TEXTBUFFER_WIDTH; x++) {
			char_t character = computer->ram->textbuffer.data[y * TEXTBUFFER_WIDTH + x];

			point_t font_cell = POINT(character.c % 16, character.c / 16); // TODO: not hardcode size

			point_t pos_in_font = POINT(font_cell.x * TEXTBUFFER_CHAR_WIDTH, font_cell.y * TEXTBUFFER_CHAR_HEIGHT);
			point_t pos_on_screen = POINT(x * TEXTBUFFER_CHAR_WIDTH, y * TEXTBUFFER_CHAR_HEIGHT);
			
			for (int i = 0; i < TEXTBUFFER_CHAR_HEIGHT; i++) {
				for (int j = 0; j < TEXTBUFFER_CHAR_WIDTH; j++) {
					// Pixel in the character
					color_t pixel = computer->ram->text_mode_font.data[(pos_in_font.y + i) * TEXT_MODE_FONT_BITMAP_WIDTH + (pos_in_font.x + j)];

					if (pixel == COLOR_BLACK) {
						computer->rgb_framebuffer[(pos_on_screen.y + i) * SCREEN_WIDTH + (pos_on_screen.x + j)] = computer->ram->palette.colors[character.bg_color];
					} else if (pixel == COLOR_WHITE) {
						computer->rgb_framebuffer[(pos_on_screen.y + i) * SCREEN_WIDTH + (pos_on_screen.x + j)] = computer->ram->palette.colors[character.fg_color];
					}
				}
			}
		}
	}

	// TODO: Cursor
	for (int y = computer->ram->terminal.cursor_y * TEXTBUFFER_CHAR_HEIGHT; y < computer->ram->terminal.cursor_y * TEXTBUFFER_CHAR_HEIGHT + TEXTBUFFER_CHAR_HEIGHT; y++) {
		for (int x = computer->ram->terminal.cursor_x * TEXTBUFFER_CHAR_WIDTH; x < computer->ram->terminal.cursor_x * TEXTBUFFER_CHAR_WIDTH + TEXTBUFFER_CHAR_WIDTH; x++) {
			computer->rgb_framebuffer[y * SCREEN_WIDTH + x] = (rgb_color_t){255, 255, 255};
		}
	}
}

void txt_putchar(ram_t *ram, uint8_t c, int x, int y, color_t bg_color, color_t fg_color) {
	if (x < 0 || x >= TEXTBUFFER_WIDTH)
		return;

	if (y < 0 || y >= TEXTBUFFER_HEIGHT)
		return;

	ram->textbuffer.data[y * TEXTBUFFER_WIDTH + x] = (char_t){
		.c = c,
		.bg_color = bg_color,
		.fg_color = fg_color,
	};
}

void txt_shift_lines_up(ram_t *ram) {
	memmove(ram->textbuffer.data, ram->textbuffer.data + TEXTBUFFER_WIDTH, (TEXTBUFFER_SIZE - TEXTBUFFER_WIDTH) * sizeof(char_t));
	
	memset(ram->textbuffer.data + (TEXTBUFFER_SIZE - TEXTBUFFER_WIDTH), 0, TEXTBUFFER_WIDTH * sizeof(char_t));
}

void txt_shift_lines_down(ram_t *ram) {
	// Move the memory
	memmove(ram->textbuffer.data + TEXTBUFFER_WIDTH, ram->textbuffer.data, (TEXTBUFFER_SIZE - TEXTBUFFER_WIDTH) * sizeof(char_t));
	
	// The old memory still exists so set to 0
	memset(ram->textbuffer.data, 0, TEXTBUFFER_WIDTH * sizeof(char_t));
}

// TODO: test this
void txt_clear(ram_t *ram) {
	memset(ram->textbuffer.data, 0, TEXTBUFFER_SIZE * sizeof(char_t));
	ram->terminal.cursor_x = 0;
	ram->terminal.cursor_y = 0;
}

static size_t _get_cursor_index(terminal_t *terminal) {
	return (size_t)terminal->cursor_y * (size_t)TEXTBUFFER_WIDTH + (size_t)terminal->cursor_x;
}

static void _scroll_if_needed(ram_t *ram) {
	if (ram->terminal.cursor_y >= TEXTBUFFER_HEIGHT) {
		txt_shift_lines_up(ram);
		ram->terminal.cursor_y = TEXTBUFFER_HEIGHT - 1;
	}
}

static void _newline(ram_t *ram) {
	ram->terminal.cursor_x = 0;
	ram->terminal.cursor_y++;

	_scroll_if_needed(ram);
}

static void _increment_cursor(ram_t *ram) {
	if (ram->terminal.cursor_x < TEXTBUFFER_WIDTH - 1) {
		ram->terminal.cursor_x++;
	} else {
		_newline(ram);
	}
}

static void _term_backspace(ram_t *ram) {
	if (ram->terminal.cursor_x > 0) {
		ram->terminal.cursor_x--;
	} else if (ram->terminal.cursor_y > 0) {
		ram->terminal.cursor_y--;
		ram->terminal.cursor_x = TEXTBUFFER_WIDTH - 1;
	}

	size_t index = _get_cursor_index(&ram->terminal);
	ram->textbuffer.data[index] = (char_t){
		.c = '\0',
		.bg_color = 0,
		.fg_color = 0,
	};
}

void term_putchar(ram_t *ram, uint8_t c, color_t bg_color, color_t fg_color) {
	if (c == '\n') {
		_newline(ram);
		return;
	}
	if (c == '\b') {
		_term_backspace(ram);
		return;
	}

	ram->textbuffer.data[_get_cursor_index(&ram->terminal)] = (char_t){
		.c = c,
		.bg_color = bg_color,
		.fg_color = fg_color,
	};

	_increment_cursor(ram);
}

void term_printc(ram_t *ram, string_t string, color_t bg_color, color_t fg_color) {
	for (size_t i = 0; i < string.len; i++) {
		term_putchar(ram, string.data[i], bg_color, fg_color);
	}
}

void term_print(ram_t *ram, string_t string) {
	term_printc(ram, string, COLOR_BLACK, COLOR_WHITE);
}

char term_getchar() {
	return input_get_as_char();
}

// TODO: use this instead of just the shell_update function
string_t term_input(string_t string) {

}



// --- Shell ---

static void _print_help(ram_t *ram) {
	term_print(ram, STR("help              print commands\n"));
	term_print(ram, STR("load   <filename> load a file\n"));
	term_print(ram, STR("save   <filename> save a file\n"));
	term_print(ram, STR("run               run the currently loaded game\n"));
	term_print(ram, STR("resume            resume the currently running game\n"));
	term_print(ram, STR("cd     <dirname>  enter a directory\n"));
	term_print(ram, STR("cd ..             enter the parent directory\n"));
	term_print(ram, STR("ls                list the files in the current directory\n"));
	term_print(ram, STR("mkdir  <dirname>  create a new directory\n"));
	term_print(ram, STR("clear             clear the screen\n"));
	term_putchar(ram, '\n', 0, 0);
	term_print(ram, STR("Press F11 to toggle fullscreen\n"));
	term_putchar(ram, '\n', 0, 0);
}

static void _execute_command(computer_t *computer, string_t input) {
	string_t_array_t arguments = string_split(get_temp_allocator(), input, ' ');

	// Print the list of splitted strings
	// for (int i = 0; i < strings.size; i++) {
	// 	printf("'");
	// 	for (int j = 0; j < strings.data[i].len; j++) {
	// 		printf("%c", strings.data[i].data[j]);
	// 	}
	// 	printf("'");
	// 	printf("\n");
	// }

	ram_t *ram = computer->ram;

	if (arguments.len == 0) {
		return;
	}

	if (string_eq(arguments.data[0], STR("help"))) {
		// Print help
		_print_help(ram);
	} 
	
	else if (string_eq(arguments.data[0], STR("load"))) {
		if (arguments.len == 2) {
			// Load the file
			string_t absolute_path = get_absolute_path(get_temp_allocator(), path_append(get_temp_allocator(), STR("discs"), path_append(get_temp_allocator(), computer->current_path, arguments.data[1])));
			if (game_load(computer, absolute_path) == 0) {
				term_print(ram, STR("Loaded "));
				term_print(ram, path_get_filename(computer->game_path));
				term_print(ram, STR("\n"));
			} else {
				term_printc(ram, STR("The given file does not exist\n"), COLOR_BLACK, COLOR_RED);
			}
		} else {
			term_printc(ram, STR("Syntax error: expected 1 argument\n"), COLOR_BLACK, COLOR_RED);
		}
	}

	else if (string_eq(arguments.data[0], STR("save"))) {
		if (arguments.len == 1) {
			// Set game name
			term_print(ram, STR("Saving "));

			// Check if untitled
			if (computer->game_path.len == 0) {
				string_t base = get_absolute_path(get_temp_allocator(), path_append(get_temp_allocator(), STR("discs"), path_append(get_temp_allocator(), computer->current_path, STR("untitled"))));
				int index = 0;
				string_t thing = base;
				
				while (path_is_file(thing)) {
					thing = string_concat(get_temp_allocator(), base, int_to_string(get_temp_allocator(), index));
					index++;
				}

				set_game_path(computer, thing);
			}
			term_print(ram, path_get_filename(computer->game_path));
			term_print(ram, STR("...\n"));

			// Save the game
			game_save(computer, computer->game_path);

			term_print(ram, STR("Completed!\n"));
		} else if (arguments.len > 1) {
			string_t absolute_path = get_absolute_path(get_temp_allocator(), path_append(get_temp_allocator(), STR("discs"), path_append(get_temp_allocator(), computer->current_path, arguments.data[1])));
			set_game_path(computer, absolute_path);
			game_save(computer, absolute_path);

		} else {
			term_printc(ram, STR("Syntax error: expected 1 argument\n"), COLOR_BLACK, COLOR_RED);
		}
	}

	else if (string_eq(arguments.data[0], STR("run"))) {
		// Run the currently loaded game
	}

	else if (string_eq(arguments.data[0], STR("resume"))) {
		// Resume the currently running but paused game
	}

	else if (string_eq(arguments.data[0], STR("cd"))) {
		if (arguments.len == 2) {
			if (string_eq(arguments.data[1], STR(".."))) {
				// Go to parent directory
				computer->current_path = path_get_parent_dir(computer->current_path);
			} else {
				// Go to second argument directory
				string_t new_current_path = path_append(get_heap_allocator(), computer->current_path, arguments.data[1]);
				// string_t full_path = path_append(get_temp_allocator(), STR("discs"), new_current_path);
				string_t absolute_path = get_absolute_path(get_heap_allocator(), path_append(get_temp_allocator(), STR("discs"), new_current_path));

				// Check if new current path actually exists
				if (path_is_dir(absolute_path)) {
					heap_dealloc(computer->current_path.data);
					computer->current_path = new_current_path;
				} else {
					term_printc(ram, STR("The given argument is not a directory\n"), COLOR_BLACK, COLOR_RED);
				}
			}
		} else {
			term_printc(ram, STR("Syntax error: expected 1 argument\n"), COLOR_BLACK, COLOR_RED);
		}
	}

	// TODO: display os absolute path instead of within the program
	else if (string_eq(arguments.data[0], STR("pwd"))) {
		term_print(ram, computer->current_path);
		term_putchar(ram, '\n', 0, 0);
	}

	else if (string_eq(arguments.data[0], STR("ls"))) {
		string_t path_with_discs = path_append(get_temp_allocator(), STR("discs"), computer->current_path);
		
		string_t_array_t directories = get_directories_in_path(get_temp_allocator(), path_with_discs);
		for (size_t i = 0; i < directories.len; i++) {
			term_printc(ram, directories.data[i], 0, 9);
			term_print(ram, STR("\n"));
		}

		string_t_array_t files = get_files_in_path(get_temp_allocator(), path_with_discs);
		for (size_t i = 0; i < files.len; i++) {
			term_printc(ram, files.data[i], COLOR_BLACK, COLOR_WHITE);
			term_print(ram, STR("\n"));
		}
	}

	else if (string_eq(arguments.data[0], STR("mkdir"))) {
		if (arguments.len == 2) {
			// Create a new directory in the discs dir
			string_t full_path = path_append(get_temp_allocator(), path_append(get_temp_allocator(), STR("discs"), computer->current_path), arguments.data[1]);
			create_directory(full_path);
		} else {
			term_printc(ram, STR("Syntax error: expected 1 argument\n"), COLOR_BLACK, COLOR_RED);
		}
	}

	else if (string_eq(arguments.data[0], STR("clear"))) {
		// Clear the textbuffer
		txt_clear(ram);
	}

	else if (string_eq(arguments.data[0], STR("export"))) {
		if (arguments.len == 2) {
			if (string_eq(path_get_filename_extension(arguments.data[1]), STR("bmp"))) {
				string_t relative_path = path_append(get_temp_allocator(), STR("exports"), arguments.data[1]);
				string_t absolute_path = get_absolute_path(get_temp_allocator(), relative_path);
				export_spritesheet(ram, absolute_path);

				term_print(ram, STR("Spritesheet exported to "));
				term_print(ram, absolute_path);
				term_print(ram, STR("\n"));
			}
		} else {
			term_printc(ram, STR("Syntax error: expected 1 argument\n"), COLOR_BLACK, COLOR_RED);
		}
	}
	
	else {
		term_printc(ram, STR("Syntax error: unrecognized command\n"), COLOR_BLACK, COLOR_RED);
	}
}

void shell_new_command(computer_t *computer) {
	computer->ram->shell.line_len = 0;
	// computer->ram->shell.command_history_index = computer->ram->shell.command_history.len - 1;
	computer->ram->shell.command_history_index = computer->ram->shell.command_history.len;
	term_printc(computer->ram, computer->current_path, 0, 7);
	term_printc(computer->ram, STR(">"), 0, 8);
}

static void _print_intro(ram_t *ram) {
// 	term_printc(ram, STR("      _            ___  _____  \n\
//      (_)          / _ \\| ____| \n\
//   _____ _ __   __| (_) | |__   \n\
//  |_  / | '_ \\ / __\\__, |___ \\  \n\
//   / /| | | | | (__  / / ___) | \n\
//  /___|_|_| |_|\\___|/_/ |____/  \n\
//                               \n\
//                               \n"), 0, 12);

	term_printc(ram, STR("\
                                _..._         .----.     .----------. \n\
                             .-'_..._''.    .   _   \\   /          /  \n\
          .--.   _..._     .' .'      '.\\  /  .' )   | /   ______.'   \n\
          |__| .'     '.  / .'            |   (_.    //   /_          \n\
          .--..   .-.   .. '               \\     ,  //      '''--.    \n\
          |  ||  '   '  || |                `'-'/  /'___          `.  \n\
.--------.|  ||  |   |  || |            .-.    /  /     `'.         | \n\
|____    ||  ||  |   |  |. '            \\  '--'  /         )        | \n\
    /   / |  ||  |   |  | \\ '.          .'-....-'  ......-'        /  \n\
  .'   /  |__||  |   |  |  '. `._____.-'/          \\          _..'`   \n\
 /    /___    |  |   |  |    `-.______ /            '------'''        \n\
|         |   |  |   |  |             `                               \n\
|_________|   '--'   '--'                                             \n\
	\n"), 0, 9);

	// term_printc(ram, STR("Zinc"), 0, 7);
	// term_printc(ram, STR("95\n"), 0, 12);
	term_printc(ram, STR("Enter help for help\n"), 0, 15);
	term_putchar(ram, '\n', 0, 0);
}

// TODO: seperate shell into editor/frontend
// Also make user able to terminate the game with Ctrl+C
void shell_init(computer_t *computer) {
	_print_intro(computer->ram);
	array_init(&computer->ram->shell.command_history, get_heap_allocator());
	shell_new_command(computer);

	// TODO: remove eventually, it's just so I don't have to type it manually
	_execute_command(computer, STR("load fucking"));
	shell_new_command(computer);
}

void shell_update(computer_t *computer) {
	ram_t *ram = computer->ram;

	// Command history
	// TODO: don't print a whole new line
	string_t_array_t *command_history = &ram->shell.command_history;
	if (input_key_pressed(KEY_UP)) {
		if (command_history->len > 0 && ram->shell.command_history_index > 0) {
			ram->shell.command_history_index--;

			string_t thing = command_history->data[ram->shell.command_history_index];

			memcpy(ram->shell.line_buffer, thing.data, thing.len * sizeof(char));
			ram->shell.line_len = thing.len;

			term_putchar(ram, '\n', 0, 0);
			term_printc(computer->ram, computer->current_path, 0, 7);
			term_printc(computer->ram, STR(">"), 0, 8);
			term_print(ram, thing);
		}
	}
	if (input_key_pressed(KEY_DOWN)) {
		if (ram->shell.command_history_index < command_history->len - 1) {
			ram->shell.command_history_index++;

			string_t thing = command_history->data[ram->shell.command_history_index];

			memcpy(ram->shell.line_buffer, thing.data, thing.len * sizeof(char));
			ram->shell.line_len = thing.len;

			term_putchar(ram, '\n', 0, 0);
			term_printc(computer->ram, computer->current_path, 0, 7);
			term_printc(computer->ram, STR(">"), 0, 8);
			term_print(ram, thing);
		}
	}

	char c = term_getchar();
	if (c == '\0') {
		return;
	}
	
	if (c == '\n') {
		// We have the command
		term_putchar(ram, c, 0, 15);
		string_t input = (string_t) {
			.data = ram->shell.line_buffer,
			.len = ram->shell.line_len,
		};

		_execute_command(computer, input);

		if (input.len > 0) {
			array_push(command_history, string_copy(get_heap_allocator(), input));
		}

		shell_new_command(computer);
	} else if (c == '\b') {
		// Backspace
		if (ram->shell.line_len > 0) {
			ram->shell.line_len--;
			_term_backspace(ram);
		}
	} else if (ram->shell.line_len < 80 - 1) {
		ram->shell.line_buffer[ram->shell.line_len] = c;
		ram->shell.line_len++;
		term_putchar(ram, c, 0, 15);
	}
}

void shell_deinit(computer_t *computer) {
	array_deinit(&computer->ram->shell.command_history);
}
