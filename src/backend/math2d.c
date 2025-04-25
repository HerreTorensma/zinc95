#include "math2d.h"

bool point_in_recti(vec2i_t point, recti_t rect) {
	if (point.x >= rect.x && point.x < rect.x + rect.w && point.y >= rect.y && point.y < rect.y + rect.h) {
		return true;
	}
	return false;
}

recti_t rect_anchor(anchor_type_t anchor_type, recti_t origin, recti_t rect) {
	recti_t new_rect = {

	};

	switch (anchor_type) {
		case ANCHOR_TYPE_TOP: {
			return (recti_t){
				.x = origin.x + rect.x,
				.y = origin.y - rect.h + rect.y,
				.w = rect.w,
				.h = rect.h,
			};
		}
		case ANCHOR_TYPE_BOTTOM: {
			return (recti_t) {
				.x = origin.x + rect.x,
				.y = origin.y + rect.h + rect.y,
				.w = rect.w,
				.h = rect.h,
			};
		}
		// TODO: finish this
	}

	return new_rect;
}