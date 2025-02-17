#include <stdio.h>

#include "sdl2.h"

static bool running = true;
static int scale = 1;
static int viewport_offset_x = 0;
static int viewport_offset_y = 0;
static bool fullscreen = false;

static SDL_Window *window = NULL;
static SDL_Renderer *renderer = NULL;
static uint32_t pixels[SCREEN_WIDTH * SCREEN_HEIGHT] = {0};
static SDL_Texture *screen_texture = NULL;

typedef struct sdl2_input {
	uint8_t prev_key_state[512];
	uint8_t key_state[512];

	uint32_t prev_mouse_state;
	uint32_t mouse_state;
} sdl2_input_t;

static sdl2_input_t sdl2_input = {0};

static void resize_window() {
	int window_width, window_height;
	SDL_GetWindowSize(window, &window_width, &window_height);

	int scale_x = window_width / SCREEN_WIDTH;
	int scale_y = window_height / SCREEN_HEIGHT;

	if (scale_x < scale_y) {
		scale = scale_x;
	} else {
		scale = scale_y;
	}
	if (scale < 1) {
		scale = 1;
	}
	SDL_RenderSetScale(renderer, scale, scale);

	viewport_offset_x = ((window_width / scale) / 2) - (SCREEN_WIDTH / 2);
	viewport_offset_y = ((window_height / scale) / 2) - (SCREEN_HEIGHT / 2);

	SDL_Rect viewport_rect = (SDL_Rect){
		.x = viewport_offset_x,
		.y = viewport_offset_y,
		.w = SCREEN_WIDTH * scale,
		.h = SCREEN_HEIGHT * scale,
	};
	SDL_RenderSetViewport(renderer, &viewport_rect);

	SDL_Rect clip_rect = (SDL_Rect){
		.x = 0,
		.y = 0,
		.w = SCREEN_WIDTH,
		.h = SCREEN_HEIGHT,
	};
	SDL_RenderSetClipRect(renderer, &clip_rect);
}

static void create_screen_texture() {
	screen_texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_STREAMING, SCREEN_WIDTH, SCREEN_HEIGHT);
}

static void update_screen_texture(computer_t *computer) {
	for (int y = 0; y < SCREEN_HEIGHT; y++) {
		for (int x = 0; x < SCREEN_WIDTH; x++) {
			rgb_color_t rgb_color = computer->rgb_framebuffer[y * SCREEN_WIDTH + x];
			uint32_t sdl_color = 0;
			sdl_color = sdl_color | (rgb_color.r << 24);
			sdl_color = sdl_color | (rgb_color.g << 16);
			sdl_color = sdl_color | (rgb_color.b << 8);
			
			sdl_color |= 0x000000FF;

			pixels[y * SCREEN_WIDTH + x] = sdl_color;
		}
	}
	
	// Update SDL2 texture
	SDL_UpdateTexture(screen_texture, NULL, pixels, SCREEN_WIDTH * sizeof(uint32_t));
}

void sdl2_init(char title[], int initial_scale) {
	scale = initial_scale;

	if (SDL_Init(SDL_INIT_VIDEO) < 0) {
		printf("Failed to initialize SDL2: %s\n", SDL_GetError());
		exit(EXIT_FAILURE);
	}
	
	window = SDL_CreateWindow(
		title,
		SDL_WINDOWPOS_UNDEFINED,
		SDL_WINDOWPOS_UNDEFINED,
		SCREEN_WIDTH * scale,
		SCREEN_HEIGHT * scale,
		SDL_WINDOW_RESIZABLE
	);
	if (window == NULL) {
		printf("Failed to create SDL2 window\n");
		exit(EXIT_FAILURE);
	}
	
	renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
	if (renderer == NULL) {
		printf("Failed to create SDL2 renderer\n");
		SDL_DestroyWindow(window);
		exit(EXIT_FAILURE);
	}
	
	resize_window();

	// Create screen texture
	create_screen_texture();
}

bool sdl2_window_is_open() {
	return running;
}

void sdl2_get_mouse_pos(int *x, int *y) {
	int sdl_x, sdl_y;
	SDL_GetMouseState(&sdl_x, &sdl_y);

	*x = (sdl_x - viewport_offset_x * scale) / scale;
	*y = (sdl_y - viewport_offset_y * scale) / scale;
}

void sdl2_tick(computer_t *computer) {
	SDL_Event event;
	while (SDL_PollEvent(&event)) {
		if (event.type == SDL_QUIT) {
			running = false;
		}

		if (event.type == SDL_WINDOWEVENT && event.window.event == SDL_WINDOWEVENT_RESIZED) {
			resize_window();
		}

		if (event.type == SDL_KEYDOWN) {
			if (event.key.keysym.sym == SDLK_F11) {
				fullscreen = !fullscreen;
				if (fullscreen) {
					SDL_SetWindowFullscreen(window, SDL_WINDOW_FULLSCREEN_DESKTOP);
				} else {
					SDL_SetWindowFullscreen(window, 0);
				}
			}
		}
	}

	sdl2_input_update();
}

void sdl2_render(computer_t *computer) {
	SDL_SetRenderDrawColor(renderer, 0, 170, 170, 255);
	SDL_RenderClear(renderer);

	update_screen_texture(computer);
	SDL_RenderCopy(renderer, screen_texture, NULL, &(SDL_Rect){0, 0, SCREEN_WIDTH, SCREEN_HEIGHT});

	SDL_RenderPresent(renderer);
}

void sdl2_tick_end() {
	SDL_Delay(1000/FPS);
}

void sdl2_quit() {
	SDL_DestroyRenderer(renderer);
	SDL_DestroyWindow(window);
	SDL_Quit();
}

static SDL_Scancode key_to_sdl2_scancode(key_t key) {
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
		default: return SDL_SCANCODE_UNKNOWN;
	}
}

static int mouse_button_to_sdl2_button(mouse_button_t button) {
	switch (button) {
		case MOUSE_BUTTON_LEFT: return 1;
		case MOUSE_BUTTON_MIDDLE: return 2;
		case MOUSE_BUTTON_RIGHT: return 3;
	}
}

void sdl2_input_update() {
	memcpy(sdl2_input.prev_key_state, sdl2_input.key_state, 256 * sizeof(uint8_t));

	const uint8_t *state = SDL_GetKeyboardState(NULL);
	memcpy(sdl2_input.key_state, state, 256 * sizeof(uint8_t));

	sdl2_input.prev_mouse_state = sdl2_input.mouse_state;
	
	int x, y;
	sdl2_input.mouse_state = SDL_GetMouseState(&x, &y);
}

bool sdl2_input_key_pressed(key_t key) {
	SDL_Scancode scancode = key_to_sdl2_scancode(key);

	if (sdl2_input.key_state[scancode] && !sdl2_input.prev_key_state[scancode]) {
		return true;
	}
	return false;
}

bool sdl2_input_key_held(key_t key) {
	SDL_Scancode scancode = key_to_sdl2_scancode(key);

	if (sdl2_input.key_state[scancode]) {
		return true;
	}
	return false;
}

bool sdl2_input_key_released(key_t key) {
	SDL_Scancode scancode = key_to_sdl2_scancode(key);

	if (!sdl2_input.key_state[scancode] && sdl2_input.prev_key_state[scancode]) {
		return true;
	}
	return false;
}

bool sdl2_input_mouse_button_pressed(mouse_button_t button) {
	int sdl2_button = mouse_button_to_sdl2_button(button);

	if (sdl2_input.mouse_state & SDL_BUTTON(sdl2_button) && !(sdl2_input.prev_mouse_state & SDL_BUTTON(sdl2_button))) {
		return true;
	}
	return false;
}

bool sdl2_input_mouse_button_released(mouse_button_t button) {
	int sdl2_button = mouse_button_to_sdl2_button(button);

	if (!(sdl2_input.mouse_state & SDL_BUTTON(sdl2_button)) && sdl2_input.prev_mouse_state & SDL_BUTTON(sdl2_button)) {
		return true;
	}
	return false;
}

bool sdl2_input_mouse_button_held(mouse_button_t button) {
	int sdl2_button = mouse_button_to_sdl2_button(button);

	if (sdl2_input.mouse_state & SDL_BUTTON(sdl2_button)) {
		return true;
	}
	return false;
}