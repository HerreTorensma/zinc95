#include <stdio.h>

#include "sdl2.h"

static bool _running = true;
static int _scale = 1;
static int _viewport_offset_x = 0;
static int _viewport_offset_y = 0;
static bool _fullscreen = false;

static SDL_Window *_window = NULL;
static SDL_Renderer *_renderer = NULL;
static uint32_t _pixels[SCREEN_WIDTH * SCREEN_HEIGHT] = {0};
static SDL_Texture *_screen_texture = NULL;

// -1 is up, 0 is none, 1 is down
static int _scroll_state = 0;

static uint32_t _frame_start_ticks = 0;

typedef struct sdl2_input {
	uint8_t prev_key_state[512];
	uint8_t key_state[512];

	uint32_t prev_mouse_state;
	uint32_t mouse_state;
} sdl2_input_t;

static sdl2_input_t _sdl2_input = {0};

static void _resize_window() {
	int window_width, window_height;
	SDL_GetWindowSize(_window, &window_width, &window_height);

	int scale_x = window_width / SCREEN_WIDTH;
	int scale_y = window_height / SCREEN_HEIGHT;

	if (scale_x < scale_y) {
		_scale = scale_x;
	} else {
		_scale = scale_y;
	}
	if (_scale < 1) {
		_scale = 1;
	}
	SDL_RenderSetScale(_renderer, _scale, _scale);

	_viewport_offset_x = ((window_width / _scale) / 2) - (SCREEN_WIDTH / 2);
	_viewport_offset_y = ((window_height / _scale) / 2) - (SCREEN_HEIGHT / 2);

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
		SDL_WINDOW_RESIZABLE
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

void sdl2_get_mouse_pos(int *x, int *y) {
	int sdl_x, sdl_y;
	SDL_GetMouseState(&sdl_x, &sdl_y);

	*x = (sdl_x - _viewport_offset_x * _scale) / _scale;
	*y = (sdl_y - _viewport_offset_y * _scale) / _scale;
}

void sdl2_tick_start(computer_t *computer) {
	_frame_start_ticks = SDL_GetTicks();

	_scroll_state = 0;

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

		// Check scrolling
		if (event.type == SDL_MOUSEWHEEL) {
			if (event.wheel.y > 0) {
				_scroll_state = -1;
			} else if (event.wheel.y < 0) {
				_scroll_state = 1;
			} else {
				_scroll_state = 0;
			}
		}
	}

	sdl2_input_update();
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

static SDL_Scancode _key_to_sdl2_scancode(zinc_key_t key) {
	switch (key) {
		case KEY_A: return SDL_SCANCODE_A;
		case KEY_B: return SDL_SCANCODE_B;
		case KEY_C: return SDL_SCANCODE_C;
		case KEY_D: return SDL_SCANCODE_D;
		case KEY_E: return SDL_SCANCODE_E;
		case KEY_F: return SDL_SCANCODE_F;
		case KEY_G: return SDL_SCANCODE_G;
		case KEY_H: return SDL_SCANCODE_H;
		case KEY_I: return SDL_SCANCODE_I;
		case KEY_J: return SDL_SCANCODE_J;
		case KEY_K: return SDL_SCANCODE_K;
		case KEY_L: return SDL_SCANCODE_L;
		case KEY_M: return SDL_SCANCODE_M;
		case KEY_N: return SDL_SCANCODE_N;
		case KEY_O: return SDL_SCANCODE_O;
		case KEY_P: return SDL_SCANCODE_P;
		case KEY_Q: return SDL_SCANCODE_Q;
		case KEY_R: return SDL_SCANCODE_R;
		case KEY_S: return SDL_SCANCODE_S;
		case KEY_T: return SDL_SCANCODE_T;
		case KEY_U: return SDL_SCANCODE_U;
		case KEY_V: return SDL_SCANCODE_V;
		case KEY_W: return SDL_SCANCODE_W;
		case KEY_X: return SDL_SCANCODE_X;
		case KEY_Y: return SDL_SCANCODE_Y;
		case KEY_Z: return SDL_SCANCODE_Z;

		case KEY_0: return SDL_SCANCODE_0;
		case KEY_1: return SDL_SCANCODE_1;
		case KEY_2: return SDL_SCANCODE_2;
		case KEY_3: return SDL_SCANCODE_3;
		case KEY_4: return SDL_SCANCODE_4;
		case KEY_5: return SDL_SCANCODE_5;
		case KEY_6: return SDL_SCANCODE_6;
		case KEY_7: return SDL_SCANCODE_7;
		case KEY_8: return SDL_SCANCODE_8;
		case KEY_9: return SDL_SCANCODE_9;

		case KEY_MINUS: return SDL_SCANCODE_MINUS;
		case KEY_EQUALS: return SDL_SCANCODE_EQUALS;
		case KEY_LEFTBRACKET: return SDL_SCANCODE_LEFTBRACKET;
		case KEY_RIGHTBRACKET: return SDL_SCANCODE_RIGHTBRACKET;
		case KEY_BACKSLASH: return SDL_SCANCODE_BACKSLASH;
		case KEY_SEMICOLON: return SDL_SCANCODE_SEMICOLON;
		case KEY_APOSTROPHE: return SDL_SCANCODE_APOSTROPHE;
		case KEY_GRAVE: return SDL_SCANCODE_GRAVE;
		case KEY_COMMA: return SDL_SCANCODE_COMMA;
		case KEY_PERIOD: return SDL_SCANCODE_PERIOD;
		case KEY_SLASH: return SDL_SCANCODE_SLASH;

		case KEY_SPACE: return SDL_SCANCODE_SPACE;
		case KEY_TAB: return SDL_SCANCODE_TAB;
		case KEY_RETURN: return SDL_SCANCODE_RETURN;
		case KEY_BACKSPACE: return SDL_SCANCODE_BACKSPACE;
		case KEY_DELETE: return SDL_SCANCODE_DELETE;
		case KEY_INSERT: return SDL_SCANCODE_INSERT;
		case KEY_PAGEUP: return SDL_SCANCODE_PAGEUP;
		case KEY_PAGEDOWN: return SDL_SCANCODE_PAGEDOWN;
		case KEY_HOME: return SDL_SCANCODE_HOME;
		case KEY_END: return SDL_SCANCODE_END;
		case KEY_UP: return SDL_SCANCODE_UP;
		case KEY_DOWN: return SDL_SCANCODE_DOWN;
		case KEY_LEFT: return SDL_SCANCODE_LEFT;
		case KEY_RIGHT: return SDL_SCANCODE_RIGHT;
		case KEY_CAPSLOCK: return SDL_SCANCODE_CAPSLOCK;

		case KEY_LCTRL: return SDL_SCANCODE_LCTRL;
		case KEY_LSHIFT: return SDL_SCANCODE_LSHIFT;
		case KEY_LALT: return SDL_SCANCODE_LALT;
		case KEY_RCTRL: return SDL_SCANCODE_RCTRL;
		case KEY_RSHIFT: return SDL_SCANCODE_RSHIFT;
		case KEY_RALT: return SDL_SCANCODE_RALT;

		case KEY_ESC: return SDL_SCANCODE_ESCAPE;
		case KEY_F1: return SDL_SCANCODE_F1;
		case KEY_F2: return SDL_SCANCODE_F2;
		case KEY_F3: return SDL_SCANCODE_F3;
		case KEY_F4: return SDL_SCANCODE_F4;
		case KEY_F5: return SDL_SCANCODE_F5;
		case KEY_F6: return SDL_SCANCODE_F6;
		case KEY_F7: return SDL_SCANCODE_F7;
		case KEY_F8: return SDL_SCANCODE_F8;
		case KEY_F9: return SDL_SCANCODE_F9;
		case KEY_F10: return SDL_SCANCODE_F10;
		case KEY_F11: return SDL_SCANCODE_F11;
		case KEY_F12: return SDL_SCANCODE_F12;

		case KEY_NUM0: return SDL_SCANCODE_KP_0;
		case KEY_NUM1: return SDL_SCANCODE_KP_1;
		case KEY_NUM2: return SDL_SCANCODE_KP_2;
		case KEY_NUM3: return SDL_SCANCODE_KP_3;
		case KEY_NUM4: return SDL_SCANCODE_KP_4;
		case KEY_NUM5: return SDL_SCANCODE_KP_5;
		case KEY_NUM6: return SDL_SCANCODE_KP_6;
		case KEY_NUM7: return SDL_SCANCODE_KP_7;
		case KEY_NUM8: return SDL_SCANCODE_KP_8;
		case KEY_NUM9: return SDL_SCANCODE_KP_9;
		case KEY_NUMPLUS: return SDL_SCANCODE_KP_PLUS;
		case KEY_NUMMINUS: return SDL_SCANCODE_KP_MINUS;
		case KEY_NUMMULTIPLY: return SDL_SCANCODE_KP_MULTIPLY;
		case KEY_NUMDIVIDE: return SDL_SCANCODE_KP_DIVIDE;
		case KEY_NUMENTER: return SDL_SCANCODE_KP_ENTER;
		case KEY_NUMPERIOD: return SDL_SCANCODE_KP_PERIOD;

		default: return SDL_SCANCODE_UNKNOWN;
	}
}

static int _mouse_button_to_sdl2_button(mouse_button_t gui_button) {
	switch (gui_button) {
		case MOUSE_BUTTON_LEFT: return 1;
		case MOUSE_BUTTON_MIDDLE: return 2;
		case MOUSE_BUTTON_RIGHT: return 3;
		default: return 0;
	}
}

void sdl2_input_update() {
	memcpy(_sdl2_input.prev_key_state, _sdl2_input.key_state, 256 * sizeof(uint8_t));

	const uint8_t *state = SDL_GetKeyboardState(NULL);
	memcpy(_sdl2_input.key_state, state, 256 * sizeof(uint8_t));

	_sdl2_input.prev_mouse_state = _sdl2_input.mouse_state;
	
	int x, y;
	_sdl2_input.mouse_state = SDL_GetMouseState(&x, &y);
}

bool sdl2_input_key_pressed(zinc_key_t key) {
	SDL_Scancode scancode = _key_to_sdl2_scancode(key);

	if (_sdl2_input.key_state[scancode] && !_sdl2_input.prev_key_state[scancode]) {
		return true;
	}
	return false;
}

bool sdl2_input_key_held(zinc_key_t key) {
	SDL_Scancode scancode = _key_to_sdl2_scancode(key);

	if (_sdl2_input.key_state[scancode]) {
		return true;
	}
	return false;
}

bool sdl2_input_key_released(zinc_key_t key) {
	SDL_Scancode scancode = _key_to_sdl2_scancode(key);

	if (!_sdl2_input.key_state[scancode] && _sdl2_input.prev_key_state[scancode]) {
		return true;
	}
	return false;
}

bool sdl2_input_mouse_button_pressed(mouse_button_t gui_button) {
	int sdl2_button = _mouse_button_to_sdl2_button(gui_button);

	if (_sdl2_input.mouse_state & SDL_BUTTON(sdl2_button) && !(_sdl2_input.prev_mouse_state & SDL_BUTTON(sdl2_button))) {
		return true;
	}
	return false;
}

bool sdl2_input_mouse_button_released(mouse_button_t gui_button) {
	int sdl2_button = _mouse_button_to_sdl2_button(gui_button);

	if (!(_sdl2_input.mouse_state & SDL_BUTTON(sdl2_button)) && _sdl2_input.prev_mouse_state & SDL_BUTTON(sdl2_button)) {
		return true;
	}
	return false;
}

bool sdl2_input_mouse_button_held(mouse_button_t gui_button) {
	int sdl2_button = _mouse_button_to_sdl2_button(gui_button);

	if (_sdl2_input.mouse_state & SDL_BUTTON(sdl2_button)) {
		return true;
	}
	return false;
}

bool sdl2_input_mouse_scrolled(scroll_dir_t direction) {
	return _scroll_state == direction;
}

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

void sdl2_set_clipboard_text(char *text) {
	SDL_SetClipboardText(text);
}

string_t sdl2_get_clipboard_text(allocator_t allocator) {
	char *text = SDL_GetClipboardText();
	string_t string = STR(text);
	return string_copy(allocator, string);
}