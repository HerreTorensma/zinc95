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

	ANCHOR_TYPE_TOPLEFT,
	ANCHOR_TYPE_TOPRIGHT,
	ANCHOR_TYPE_BOTTOMLEFT,
	ANCHOR_TYPE_BOTTOMRIGHT,

	ANCHOR_TYPE_CENTER,
} anchor_type_t;

// rect_t rect_anchor(anchor_type_t anchor_type, rect_t origin, rect_t rect, int margin_x, int margin_y);

// Clips the given rect to be contained within the container
// Currently if the given rect is completely outside the container, the width and height are just set to 0 and the rect is not moved
// I might change this later if necessary 
rect_t rect_clip(rect_t container, rect_t rect);

typedef union vec2 {
	float x;
	float y;
} vec2_t;

rect_t rect_put_above(rect_t anchor_rect, rect_t rect, int margin);

rect_t rect_put_below(rect_t anchor_rect, rect_t rect, int margin);

rect_t rect_put_left(rect_t anchor_rect, rect_t rect, int margin);

rect_t rect_put_right(rect_t anchor_rect, rect_t rect, int margin);

rect_t rect_anchor_top(rect_t anchor_rect, rect_t rect, int margin);

rect_t rect_anchor_bottom(rect_t anchor_rect, rect_t rect, int margin);

rect_t rect_anchor_left(rect_t anchor_rect, rect_t rect, int margin);

rect_t rect_anchor_right(rect_t anchor_rect, rect_t rect, int margin);

// TODO: I also need actual anchoring functions for e.g. the menu bar, to position a rect within another rect 
// Or do I even need these at all?