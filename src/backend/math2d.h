/*
Math2D

Vector and rect implemention
*/

#pragma once

#include <stdbool.h>

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

void rect_print(rect_t rect);

// Check if an x, y coordinate is inside a given rect
bool point_in_rect(point_t point, rect_t rect);

// Clips the given rect to be contained within the container
// Currently if the given rect is completely outside the container, the width and height are just set to 0 and the rect is not moved
// I might change this later if necessary 
rect_t rect_clip(rect_t container, rect_t rect);

typedef union vec2 {
	float x;
	float y;
} vec2_t;

rect_t rect_from_2_points(point_t start, point_t end);