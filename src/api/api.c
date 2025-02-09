#include "api.h"

#include <stdio.h>
#include <math.h>

void api_cls(computer_t *computer, int color) {
	for (int y = 0; y < SCREEN_HEIGHT; y++) {
		for (int x = 0; x < SCREEN_WIDTH; x++) {
			computer->ram.framebuffer.data[y * SCREEN_WIDTH + x] = color;
		}
	}
}

// Using Bresemham's line algorithm
void api_line(computer_t *computer, int x1, int y1, int x2, int y2, int color) {
	int dx = abs(x2 - x1);
	int dy = abs(y2 - y1);
	int step_x = (x1 < x2) ? 1 : -1;
	int step_y = (y1 < y2) ? 1 : -1;
	int error = dx - dy;

	while (true) {
		set_pixel(computer, x1, y1, color);
		
		if (x1 == x2 && y1 == y2) break;

		int e2 = 2 * error;
		if (e2 > -dy) {
			error -= dy;
			x1 += step_x;
		}
		if (e2 < dx) {
			error += dx;
			y1 += step_y;
		}
	}
}

void api_rect(computer_t *computer, int x, int y, int w, int h, int color) {
	for (int j = x; j < x+w; j++) {
		set_pixel(computer, j, y, color);
	}

	for (int j = x; j < x+w; j++) {
		set_pixel(computer, j, y+h-1, color);
	}

	for (int i = y; i < y+h; i++) {
		set_pixel(computer, x, i, color);
	}

	for (int i = y; i < y+h; i++) {
		set_pixel(computer, x+w - 1, i, color);
	}
}

void api_rectf(computer_t *computer, int x, int y, int w, int h, int color) {
	// Using i and j to avoid conflict with the x and y parameters
	for (int i = y; i < y+h; i++) {
		for (int j = x; j < x+w; j++) {
			set_pixel(computer, j, i, color);
		}
	}
}