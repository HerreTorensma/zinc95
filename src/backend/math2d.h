/*
Math2D

Vector and rect implemention
*/

#pragma once

#include <stdbool.h>

// typedef union vec2 {
// 	struct {
// 		float x;
// 		float y;
// 	};

// 	float data[2];
// } vec2_t;

// typedef struct vec2i {
// 	struct {
// 		int x;
// 		int y;
// 	};

// 	int data[2];
// } vec2i_t;

typedef union vec2 {
	float x;
	float y;
} vec2_t;

typedef struct vec2i {
	int x;
	int y;
} vec2i_t;

// TODO: rename to rect_t and rectf_t
typedef union recti {
	struct {
		vec2i_t pos;
		vec2i_t size;
	};

	struct {
		int x;
		int y;
		int w;
		int h;
	};

	int data[4];
} recti_t;

#define RECTI(x, y, w, h) ((recti_t){.x = (x), .y = (y), .w = (w), .h = (h)});

/*
typedef union rect {
	struct {
		vec2_t pos;
		vec2_t size;
	};

	struct {
		float x;
		float y;
		float width;
		float height;
	};

	float data[4];
} rect_t;

// Convert recti to rect
rect_t recti_to_rect(recti_t integer_rect);

// Convert rect to recti
recti_t rect_to_recti(rect_t rect);
*/

// Check if an x, y coordinate is inside a given rect
bool point_in_recti(vec2i_t point, recti_t rect);

typedef enum anchor_type {
	ANCHOR_TYPE_TOP,
	ANCHOR_TYPE_BOTTOM,
	ANCHOR_TYPE_LEFT,
	ANCHOR_TYPE_RIGHT,
} anchor_type_t;

recti_t rect_anchor(anchor_type_t anchor_type, recti_t origin, recti_t rect);