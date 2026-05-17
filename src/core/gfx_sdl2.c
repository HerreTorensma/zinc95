#include "gfx.h"

#include <SDL2/SDL.h>

void gfx_load_surface(palette_t *palette, surface_t surface, string_t path) {
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

void gfx_save_surface(palette_t *palette, surface_t surface, string_t path) {
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