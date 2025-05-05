#include "math2d.h"

bool point_in_rect(point_t point, rect_t rect) {
	if (point.x >= rect.x && point.x < rect.x + rect.w && point.y >= rect.y && point.y < rect.y + rect.h) {
		return true;
	}
	return false;
}

// TODO: properly test this function
rect_t rect_clip(rect_t container, rect_t rect) {
	if (rect.x < container.x) {
		rect.x = container.x;
		rect.w -= container.x - rect.x;
	}
	if (rect.y < container.y) {
		rect.y = container.y;
		rect.h -= container.y - rect.y;
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
