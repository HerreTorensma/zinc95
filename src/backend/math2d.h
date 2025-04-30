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
		point_t pos;
		point_t size;
	};

	struct {
		int x;
		int y;
		int w;
		int h;
	};
} rect_t;

#define RECT(_x, _y, _w, _h) ((rect_t){.x = _x, .y = _y, .w = _w, .h = _h})

// Check if an x, y coordinate is inside a given rect
bool point_in_rect(point_t point, rect_t rect);

typedef enum anchor_type {
	ANCHOR_TYPE_TOP,
	ANCHOR_TYPE_BOTTOM,
	ANCHOR_TYPE_LEFT,
	ANCHOR_TYPE_RIGHT,
} anchor_type_t;

rect_t rect_anchor(anchor_type_t anchor_type, rect_t origin, rect_t rect, int margin_x, int margin_y);

// Clips the given rect to be contained within the container
// Currently if the given rect is completely outside the container, the width and height are just set to 0 and the rect is not moved
// I might change this later if necessary 
rect_t rect_clip(rect_t container, rect_t rect);

typedef union vec2 {
	float x;
	float y;
} vec2_t;