#include "math2d.h"

bool point_in_rect(point_t point, rect_t rect) {
	if (point.x >= rect.x && point.x < rect.x + rect.w && point.y >= rect.y && point.y < rect.y + rect.h) {
		return true;
	}
	return false;
}

// Only changes y coords
rect_t rect_put_above(rect_t anchor_rect, rect_t rect, int margin) {
	rect.y = anchor_rect.y - rect.h - margin;
	return rect;
}

// Only changes y coords
rect_t rect_put_below(rect_t anchor_rect, rect_t rect, int margin) {
	rect.y = (anchor_rect.y + anchor_rect.h) + margin;
	return rect;
}

// Only changes x coords
rect_t rect_put_left(rect_t anchor_rect, rect_t rect, int margin) {
	rect.x = anchor_rect.x - rect.w - margin;
	return rect;
}

// Only changes x coords
rect_t rect_put_right(rect_t anchor_rect, rect_t rect, int margin) {
	rect.x = (anchor_rect.x + anchor_rect.w) + margin;
	return rect;
}

rect_t rect_anchor_top(rect_t anchor_rect, rect_t rect, int margin) {
	rect.y = anchor_rect.y + margin;
	return rect;
}

rect_t rect_anchor_bottom(rect_t anchor_rect, rect_t rect, int margin) {
	rect.y = (anchor_rect.y + anchor_rect.h) - rect.h - margin;
	return rect;
}

rect_t rect_anchor_left(rect_t anchor_rect, rect_t rect, int margin) {
	rect.y = anchor_rect.y + margin;
	return rect;
}

rect_t rect_anchor_right(rect_t anchor_rect, rect_t rect, int margin) {
	rect.y = (anchor_rect.y + anchor_rect.h) - rect.w - margin;
	return rect;
}

// rect_t rect_anchor(anchor_type_t anchor_type, rect_t origin, rect_t rect, int margin_x, int margin_y) {
// 	switch (anchor_type) {
// 		case ANCHOR_TYPE_TOP: {
// 			return (rect_t){
// 				.x = origin.x + rect.x + margin_x,
// 				.y = origin.y - rect.h + rect.y + margin_y,
// 				.w = rect.w,
// 				.h = rect.h,
// 			};
// 		}
// 		case ANCHOR_TYPE_BOTTOM: {
// 			return (rect_t) {
// 				.x = origin.x + rect.x - margin_x,
// 				.y = origin.y + rect.h + rect.y - margin_y,
// 				.w = rect.w,
// 				.h = rect.h,
// 			};
// 		}
// 		case ANCHOR_TYPE_LEFT: {
// 			return (rect_t){
// 				.x = 0,
// 				.y = 0,
// 				.w = 0,
// 				.h = 0,
// 			};
// 		}
// 		case ANCHOR_TYPE_RIGHT: {
// 			return (rect_t){
// 				.x = 0,
// 				.y = 0,
// 				.w = 0,
// 				.h = 0,
// 			};
// 		}

// 		case ANCHOR_TYPE_TOPLEFT: {
// 			return (rect_t){
// 				.x = 0,
// 				.y = 0,
// 				.w = 0,
// 				.h = 0,
// 			};
// 		}
// 		case ANCHOR_TYPE_TOPRIGHT: {
// 			return (rect_t){
// 				.x = 0,
// 				.y = 0,
// 				.w = 0,
// 				.h = 0,
// 			};
// 		}
// 		case ANCHOR_TYPE_BOTTOMLEFT: {
// 			return (rect_t){
// 				.x = 0,
// 				.y = 0,
// 				.w = 0,
// 				.h = 0,
// 			};
// 		}
// 		case ANCHOR_TYPE_BOTTOMRIGHT: {
// 			return (rect_t){
// 				.x = 0,
// 				.y = 0,
// 				.w = 0,
// 				.h = 0,
// 			};
// 		}

// 		case ANCHOR_TYPE_CENTER: {
// 			return (rect_t){
// 				.x = 0,
// 				.y = 0,
// 				.w = 0,
// 				.h = 0,
// 			};
// 		}
// 	}

// 	return (rect_t){0};
// }

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
