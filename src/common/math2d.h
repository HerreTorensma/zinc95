/*
Math2D

Vector and rect implemention
*/

#pragma once

#include <stdbool.h>

#define MIN(a, b) (a < b ? a : b)
#define MAX(a, b) (a > b ? a : b)

static inline int clamp_int(int value, int min, int max) {
	if (value < min) return min;
	if (value > max) return max;
	return value;
}

typedef struct point {
	int x;
	int y;
} point_t;

#define POINT(_x, _y) ((point_t){.x = _x, .y = _y})

typedef union rect {
	struct {
		int x;
		int y;
		int w;
		int h;
	};

	struct {
		point_t pos;
		point_t size;
	};
} rect_t;

#define RECT(_x, _y, _w, _h) ((rect_t){.x = _x, .y = _y, .w = _w, .h = _h})
#define RECT_EMPTY ((rect_t){0, 0, 0, 0})

void rect_print(rect_t rect);

bool rect_in_rect(rect_t container, rect_t containee);

// Check if an x, y coordinate is inside a given rect
bool point_in_rect(point_t point, rect_t rect);

bool point_in_circle(point_t point, point_t circle_center, int circle_radius);

// Clips the given rect to be contained within the container
// Currently if the given rect is completely outside the container, the width and height are just set to 0 and the rect is not moved
// I might change this later if necessary 
rect_t rect_clip(rect_t container, rect_t rect);

typedef union vec2 {
	float x;
	float y;
} vec2_t;

// TODO: put in lua api

// Includes the bottom left pixel
rect_t rect_from_2_points(point_t start, point_t end);

point_t rect_topleft(rect_t rect);

point_t rect_bottomright(rect_t rect);

rect_t rect_reset_origin(rect_t rect);

typedef struct camera {
	point_t pos;
	float zoom;
	point_t screen_origin; // Screen space coordinate that the camera contents are drawn at
} camera_t;

// Convert world coordinates to screen coordinates
point_t cam_world_to_screen(camera_t *camera, point_t world);

// Convert screen coordinates to world coordinates
point_t cam_screen_to_world(camera_t *camera, point_t screen);

// Snap a world point to the grid (cell size in pixels)
point_t snap_to_grid(point_t world, int cell_w, int cell_h);

// Converts screen coordinates to tile indices in a layer (rect_in_tiles is in tiles)
point_t cam_screen_to_tile(camera_t *camera, point_t screen, rect_t rect_in_tiles, int cell_w, int cell_h);

// Converts tile coordinates back to screen coordinates (rect_in_tiles is in tiles)
point_t cam_tile_to_screen(camera_t *camera, point_t tile, rect_t rect_in_tiles, int cell_w, int cell_h);
