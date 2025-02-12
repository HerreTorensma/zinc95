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