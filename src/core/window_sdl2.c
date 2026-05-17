#include "window.h"

#include <SDL2/SDL.h>

static bool _running = true;
static int _scale = 1;
static int _viewport_offset_x = 0;
static int _viewport_offset_y = 0;
static bool _fullscreen = false;

static SDL_Window *_window = NULL;
static SDL_Renderer *_renderer = NULL;
static uint32_t _pixels[SCREEN_WIDTH * SCREEN_HEIGHT] = {0};
static SDL_Texture *_screen_texture = NULL;

static uint32_t _frame_start_ticks = 0;

static float _dpi_scale_x = 1.0f;
static float _dpi_scale_y = 1.0f;

static void _resize_window() {
	int window_width, window_height;
	SDL_GetWindowSize(_window, &window_width, &window_height);

	int renderer_width, renderer_height;
	SDL_GetRendererOutputSize(_renderer, &renderer_width, &renderer_height);

	_dpi_scale_x = (float)renderer_width / (float)window_width;
	_dpi_scale_y = (float)renderer_height / (float)window_height;

	{
		int scale_x = renderer_width / SCREEN_WIDTH;
		int scale_y = renderer_height / SCREEN_HEIGHT;
	
		if (scale_x < scale_y) {
			_scale = scale_x;
		} else {
			_scale = scale_y;
		}
		if (_scale < 1) {
			_scale = 1;
		}
		SDL_RenderSetScale(_renderer, _scale, _scale);
	}
	
	_viewport_offset_x = ((renderer_width / _scale) / 2) - (SCREEN_WIDTH / 2);
	_viewport_offset_y = ((renderer_height / _scale) / 2) - (SCREEN_HEIGHT / 2);

	SDL_Rect viewport_rect = (SDL_Rect){
		.x = _viewport_offset_x,
		.y = _viewport_offset_y,
		.w = SCREEN_WIDTH * _scale,
		.h = SCREEN_HEIGHT * _scale,
	};
	SDL_RenderSetViewport(_renderer, &viewport_rect);

	SDL_Rect clip_rect = (SDL_Rect){
		.x = 0,
		.y = 0,
		.w = SCREEN_WIDTH,
		.h = SCREEN_HEIGHT,
	};
	SDL_RenderSetClipRect(_renderer, &clip_rect);
}

static void _create_screen_texture() {
	_screen_texture = SDL_CreateTexture(_renderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_STREAMING, SCREEN_WIDTH, SCREEN_HEIGHT);
}

static void _update_screen_texture(computer_t *computer) {
	for (int y = 0; y < SCREEN_HEIGHT; y++) {
		for (int x = 0; x < SCREEN_WIDTH; x++) {
			rgb_color_t rgb_color = computer->rgb_framebuffer[y * SCREEN_WIDTH + x];
			uint32_t sdl_color = 0;
			sdl_color = sdl_color | (rgb_color.r << 24);
			sdl_color = sdl_color | (rgb_color.g << 16);
			sdl_color = sdl_color | (rgb_color.b << 8);
			
			sdl_color |= 0x000000FF;

			_pixels[y * SCREEN_WIDTH + x] = sdl_color;
		}
	}
	
	// Update SDL2 texture
	SDL_UpdateTexture(_screen_texture, NULL, _pixels, SCREEN_WIDTH * sizeof(uint32_t));
}

void window_init(string_t title, int initial_scale) {
	_scale = initial_scale;

	if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO) < 0) {
		printf("Failed to initialize SDL2: %s\n", SDL_GetError());
		exit(EXIT_FAILURE);
	}
	
	_window = SDL_CreateWindow(
		string_to_c_string(get_temp_allocator(), title),
		SDL_WINDOWPOS_UNDEFINED,
		SDL_WINDOWPOS_UNDEFINED,
		SCREEN_WIDTH * _scale,
		SCREEN_HEIGHT * _scale,
		SDL_WINDOW_RESIZABLE | SDL_WINDOW_ALLOW_HIGHDPI
	);
	if (_window == NULL) {
		printf("Failed to create SDL2 window\n");
		exit(EXIT_FAILURE);
	}
	
	_renderer = SDL_CreateRenderer(_window, -1, 0);
	if (_renderer == NULL) {
		printf("Failed to create SDL2 renderer\n");
		SDL_DestroyWindow(_window);
		exit(EXIT_FAILURE);
	}
	
	_resize_window();

	// Create screen texture
	_create_screen_texture();
}

void window_tick_start(computer_t *computer) {
	_frame_start_ticks = SDL_GetTicks();

	int scroll_state = 0;

	SDL_Event event;
	while (SDL_PollEvent(&event)) {
		if (event.type == SDL_QUIT) {
			_running = false;
		}

		if (event.type == SDL_WINDOWEVENT && event.window.event == SDL_WINDOWEVENT_RESIZED) {
			_resize_window();
		}

		if (event.type == SDL_KEYDOWN) {
			if (event.key.keysym.sym == SDLK_F11) {
				_fullscreen = !_fullscreen;
				if (_fullscreen) {
					SDL_SetWindowFullscreen(_window, SDL_WINDOW_FULLSCREEN_DESKTOP);
				} else {
					SDL_SetWindowFullscreen(_window, 0);
				}
			}
		}

		// Check file drop
		if (event.type == SDL_DROPFILE) {
			char *dropped_file = event.drop.file;
			import_file(computer, string_copy(get_temp_allocator(), STR(dropped_file)));
			SDL_free(dropped_file);
		}

		if (event.type == SDL_MOUSEWHEEL) {
			if (event.wheel.y > 0) {
				scroll_state = -1;
			} else if (event.wheel.y < 0) {
				scroll_state = 1;
			} else {
				scroll_state = 0;
			}
		}
	}

	input_update(scroll_state);
	input_core_update();
	input_set_cursor_style(CURSOR_STYLE_ARROW);
}

void window_render(computer_t *computer) {
	rgb_color_t border_color = computer->ram->palette.colors[computer->ram->border_color];
	SDL_SetRenderDrawColor(_renderer, border_color.r, border_color.g, border_color.b, 255);

	SDL_RenderClear(_renderer);

	_update_screen_texture(computer);
	SDL_RenderCopy(_renderer, _screen_texture, NULL, &(SDL_Rect){0, 0, SCREEN_WIDTH, SCREEN_HEIGHT});

	SDL_RenderPresent(_renderer);
}

void window_tick_end(computer_t *computer) {
	// Ensure the target FPS
	uint32_t frame_ticks = SDL_GetTicks() - _frame_start_ticks;

	if (FRAME_DELAY > frame_ticks) {
		SDL_Delay(FRAME_DELAY - frame_ticks);
	}

	computer->ram->ticks++;
}

bool window_is_open() {
	return _running;
}

void window_quit() {
	SDL_DestroyRenderer(_renderer);
	SDL_DestroyWindow(_window);
	SDL_Quit();
}

void set_clipboard_text(allocator_t allocator, string_t string) {
	char *c_string = string_to_c_string(allocator, string);
	SDL_SetClipboardText(c_string);
	dealloc(allocator, c_string);
}

string_t get_clipboard_text(allocator_t allocator) {
	char *text = SDL_GetClipboardText();
	string_t string = STR(text);
	return string_copy(allocator, string);
}

point_t window_get_mouse_pos() {
	int sdl_x, sdl_y;
	SDL_GetMouseState(&sdl_x, &sdl_y);

	float adjusted_x = (float)sdl_x * _dpi_scale_x;
	float adjusted_y = (float)sdl_y * _dpi_scale_y;

	return (point_t){
		.x = (adjusted_x / _scale - _viewport_offset_x),
		.y = (adjusted_y / _scale - _viewport_offset_y),
	};
}
