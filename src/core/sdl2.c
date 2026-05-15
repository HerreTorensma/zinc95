#include <SDL2/SDL_audio.h>
#include <SDL2/SDL_events.h>
#include <SDL2/SDL_mouse.h>
#include <SDL2/SDL_pixels.h>
#include <SDL2/SDL_surface.h>
#include <stdio.h>

#include "sdl2.h"
#include "gfx.h"
#include "input.h"

static bool _running = true;
static int _scale = 1;
static int _viewport_offset_x = 0;
static int _viewport_offset_y = 0;
static bool _fullscreen = false;

static SDL_Window *_window = NULL;
static SDL_Renderer *_renderer = NULL;
static uint32_t _pixels[SCREEN_WIDTH * SCREEN_HEIGHT] = {0};
static SDL_Texture *_screen_texture = NULL;

// // -1 is up, 0 is none, 1 is down
// static int _scroll_state = 0;

static uint32_t _frame_start_ticks = 0;

// static SDL_Cursor *_cursors[CURSOR_STYLE_COUNT];

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

void sdl2_init(char title[], int initial_scale) {
	_scale = initial_scale;

	if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO) < 0) {
		printf("Failed to initialize SDL2: %s\n", SDL_GetError());
		exit(EXIT_FAILURE);
	}
	
	_window = SDL_CreateWindow(
		title,
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

bool sdl2_window_is_open() {
	return _running;
}

// void sdl2_get_mouse_pos(int *x, int *y) {
// 	int sdl_x, sdl_y;
// 	SDL_GetMouseState(&sdl_x, &sdl_y);

// 	float adjusted_x = (float)sdl_x * _dpi_scale_x;
// 	float adjusted_y = (float)sdl_y * _dpi_scale_y;

// 	*x = (adjusted_x / _scale - _viewport_offset_x);
// 	*y = (adjusted_y / _scale - _viewport_offset_y);
// }

void sdl2_tick_start(computer_t *computer) {
	_frame_start_ticks = SDL_GetTicks();

	// _scroll_state = 0;

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

		// // Check scrolling
		// if (event.type == SDL_MOUSEWHEEL) {
		// 	if (event.wheel.y > 0) {
		// 		_scroll_state = -1;
		// 	} else if (event.wheel.y < 0) {
		// 		_scroll_state = 1;
		// 	} else {
		// 		_scroll_state = 0;
		// 	}
		// }

		// Check file drop
		if (event.type == SDL_DROPFILE) {
			char *dropped_file = event.drop.file;
			import_file(computer, string_copy(get_temp_allocator(), STR(dropped_file)));
			SDL_free(dropped_file);
		}
	}

	// sdl2_input_update();
}

void sdl2_render(computer_t *computer) {
	rgb_color_t border_color = computer->ram->palette.colors[computer->ram->border_color];
	SDL_SetRenderDrawColor(_renderer, border_color.r, border_color.g, border_color.b, 255);

	SDL_RenderClear(_renderer);

	_update_screen_texture(computer);
	SDL_RenderCopy(_renderer, _screen_texture, NULL, &(SDL_Rect){0, 0, SCREEN_WIDTH, SCREEN_HEIGHT});

	SDL_RenderPresent(_renderer);
}

void sdl2_tick_end() {
	// Ensure the target FPS
	uint32_t frame_ticks = SDL_GetTicks() - _frame_start_ticks;

	if (FRAME_DELAY > frame_ticks) {
		SDL_Delay(FRAME_DELAY - frame_ticks);
	}
}

void sdl2_quit() {
	SDL_DestroyRenderer(_renderer);
	SDL_DestroyWindow(_window);
	SDL_Quit();
}

// static int _mouse_button_to_sdl2_button(mouse_button_t gui_button) {
// 	switch (gui_button) {
// 		case MOUSE_BUTTON_LEFT: return 1;
// 		case MOUSE_BUTTON_MIDDLE: return 2;
// 		case MOUSE_BUTTON_RIGHT: return 3;
// 		default: return 0;
// 	}
// }

// void sdl2_input_update() {
// 	memcpy(_sdl2_input.prev_key_state, _sdl2_input.key_state, 256 * sizeof(uint8_t));

// 	const uint8_t *state = SDL_GetKeyboardState(NULL);
// 	memcpy(_sdl2_input.key_state, state, 256 * sizeof(uint8_t));

// 	_sdl2_input.prev_mouse_state = _sdl2_input.mouse_state;
	
// 	int x, y;
// 	_sdl2_input.mouse_state = SDL_GetMouseState(&x, &y);
// }

// bool sdl2_input_key_pressed(zinc_key_t key) {
// 	SDL_Scancode scancode = _key_to_sdl2_scancode(key);

// 	if (_sdl2_input.key_state[scancode] && !_sdl2_input.prev_key_state[scancode]) {
// 		return true;
// 	}
// 	return false;
// }

// bool sdl2_input_key_released(zinc_key_t key) {
// 	SDL_Scancode scancode = _key_to_sdl2_scancode(key);

// 	if (!_sdl2_input.key_state[scancode] && _sdl2_input.prev_key_state[scancode]) {
// 		return true;
// 	}
// 	return false;
// }

// bool sdl2_input_mouse_button_pressed(mouse_button_t gui_button) {
// 	int sdl2_button = _mouse_button_to_sdl2_button(gui_button);

// 	if (_sdl2_input.mouse_state & SDL_BUTTON(sdl2_button) && !(_sdl2_input.prev_mouse_state & SDL_BUTTON(sdl2_button))) {
// 		return true;
// 	}
// 	return false;
// }

// bool sdl2_input_mouse_button_released(mouse_button_t gui_button) {
// 	int sdl2_button = _mouse_button_to_sdl2_button(gui_button);

// 	if (!(_sdl2_input.mouse_state & SDL_BUTTON(sdl2_button)) && _sdl2_input.prev_mouse_state & SDL_BUTTON(sdl2_button)) {
// 		return true;
// 	}
// 	return false;
// }

// bool sdl2_input_mouse_button_held(mouse_button_t gui_button) {
// 	int sdl2_button = _mouse_button_to_sdl2_button(gui_button);

// 	if (_sdl2_input.mouse_state & SDL_BUTTON(sdl2_button)) {
// 		return true;
// 	}
// 	return false;
// }

// bool sdl2_input_mouse_scrolled(scroll_dir_t direction) {
// 	return _scroll_state == direction;
// }

void audio_update(float *buffer, int frames);

static void _sdl2_audio_callback(void *userdata, uint8_t *stream, int len) {
	float *out = (float *)stream;
	int frames = len / (sizeof(float) * CHANNELS);
	audio_update(out, frames);
}

void sdl2_audio_init(computer_t *computer) {
	SDL_AudioSpec wav_spec;

	SDL_AudioSpec spec = {
		.freq = SAMPLE_RATE,
		// .format = AUDIO_S16LSB,
		.format = AUDIO_F32SYS, // A sample is a 32 bit float
		// .channels = 1, // 1 channel for stereo audio
		.channels = 2, // 2 channels for stereo audio
		.samples = SAMPLES, // 1024 size of the audio buffer to be filled
		.callback = _sdl2_audio_callback,
	};

	if (SDL_OpenAudio(&spec, NULL) < 0) {
		printf("SDL_OpenAudio failed! SDL_Error: %s\n", SDL_GetError());
	}

	SDL_PauseAudio(0);
}

void sdl2_audio_deinit(computer_t *computer) {
	SDL_CloseAudio();
}

void sdl2_set_clipboard_text(allocator_t allocator, string_t string) {
	char *c_string = string_to_c_string(allocator, string);
	SDL_SetClipboardText(c_string);
	dealloc(allocator, c_string);
}

string_t sdl2_get_clipboard_text(allocator_t allocator) {
	char *text = SDL_GetClipboardText();
	string_t string = STR(text);
	return string_copy(allocator, string);
}

void sdl2_load_bmp_to_surface(palette_t *palette, surface_t surface, string_t path) {
	SDL_Surface *sdl_surface = SDL_LoadBMP(string_to_c_string(get_temp_allocator(), path));

	if (sdl_surface == NULL) {
		printf("Failed to load BMP: ");
		print_string(path);
		printf("\n");
		return;
	}

	for (int y = 0; y < sdl_surface->h; y++) {
		for (int x = 0; x < sdl_surface->w; x++) {
			uint32_t color_as_uint32 = *((uint32_t *)((uint8_t *)sdl_surface->pixels + (y * sdl_surface->pitch + x * sdl_surface->format->BytesPerPixel)));

			rgb_color_t rgb_color = {0};
			SDL_GetRGB(color_as_uint32, sdl_surface->format, &rgb_color.r, &rgb_color.g, &rgb_color.b);
			
			// Convert to pallete pixel
			color_t color = gfx_rgb_color_to_color(palette, rgb_color, COLOR_BLACK);
			surface.data[y * sdl_surface->w + x] = color;
		}
	}

	SDL_FreeSurface(sdl_surface);
}

void sdl2_save_surface_as_bmp(palette_t *palette, surface_t surface, string_t path) {
	SDL_Surface *sdl_surface = SDL_CreateRGBSurface(0, SPRITESHEET_WIDTH, SPRITESHEET_HEIGHT, 32, 0, 0, 0, 0);

	if (SDL_MUSTLOCK(sdl_surface)) {
		SDL_LockSurface(sdl_surface);
	}

	for (size_t y = 0; y < SPRITESHEET_HEIGHT; y++) {
		for (size_t x = 0; x < SPRITESHEET_WIDTH; x++) {
			rgb_color_t rgb_color = palette->colors[surface.data[y * SPRITESHEET_WIDTH + x]];
			
			uint32_t sdl_color = SDL_MapRGBA(
				sdl_surface->format,
				rgb_color.r,
				rgb_color.g,
				rgb_color.b,
				255
			);

			*(uint32_t *)((uint8_t *)sdl_surface->pixels + (y * sdl_surface->pitch + x * sdl_surface->format->BytesPerPixel)) = sdl_color;
		}
	}

	if (SDL_MUSTLOCK(sdl_surface)) {
		SDL_UnlockSurface(sdl_surface);
	}

	SDL_SaveBMP(sdl_surface, string_to_c_string(get_temp_allocator(), path));

	SDL_FreeSurface(sdl_surface);
}
