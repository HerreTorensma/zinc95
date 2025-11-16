#include "math2d.h"

#include <stdio.h>

void rect_print(rect_t rect) {
	printf("x: %d, y: %d, w: %d, h: %d\n", rect.x, rect.y, rect.w, rect.h);
}

bool point_in_rect(point_t point, rect_t rect) {
	if (point.x >= rect.x && point.x < rect.x + rect.w && point.y >= rect.y && point.y < rect.y + rect.h) {
		return true;
	}
	return false;
}

// TODO: properly test this function
// TODO: make CLAMP macro and use it here
rect_t rect_clip(rect_t container, rect_t rect) {
	if (rect.x < container.x) {
		int delta = container.x - rect.x;
		rect.x = container.x;
		rect.w -= delta;
	}
	if (rect.y < container.y) {
		int delta = container.y - rect.y;
		rect.y = container.y;
		rect.h -= delta;
	}

	int rect_right = rect.x + rect.w;
	int container_right = container.x + container.w;
	int rect_bottom = rect.y + rect.h;
	int container_bottom = container.y + container.h;
	
	if (rect_right > container_right) {
		rect.w = container_right - rect.x;
	}
	if (rect_bottom > container_bottom) {
		rect.h = container_bottom - rect.y;
	}

	// Make sure the width and height are not negative
	if (rect.w < 0) {
		rect.w = 0;
	}

	if (rect.h < 0) {
		rect.h = 0;
	}

	return rect;
}

// TODO: change to use MIN, MAX
rect_t rect_from_2_points(point_t start, point_t end) {
	rect_t rect = {0};
	
	if (start.x <= end.x) {
		rect.x = start.x;
		rect.w = end.x - start.x;
	} else {
		rect.x = end.x;
		rect.w = start.x - end.x;
	}

	if (start.y <= end.y) {
		rect.y = start.y;
		rect.h = end.y - start.y;
	} else {
		rect.y = end.y;
		rect.h = start.y - end.y;
	}

	return rect;
}
