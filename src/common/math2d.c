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

bool rect_in_rect(rect_t container, rect_t containee) {
	point_t top_left = containee.pos;
	
	// TODO: make function for this???
	point_t bottom_right = {
		.x = containee.pos.x + containee.w,
		.y = containee.pos.y + containee.h,
	};

	return point_in_rect(top_left, container) && point_in_rect(bottom_right, container);
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

	rect.w++;
	rect.h++;

	return rect;
}

point_t cam_world_to_screen(camera_t *camera, point_t world) {
	return (point_t){
		.x = (int)((world.x - camera->pos.x + camera->screen_origin.x / camera->zoom) * camera->zoom),
		.y = (int)((world.y - camera->pos.y + camera->screen_origin.y / camera->zoom) * camera->zoom),
	};
}

// Convert screen coordinates to world coordinates
point_t cam_screen_to_world(camera_t *camera, point_t screen) {
	return (point_t){
		.x = (int)((screen.x / camera->zoom) + camera->pos.x - camera->screen_origin.x / camera->zoom),
		.y = (int)((screen.y / camera->zoom) + camera->pos.y - camera->screen_origin.y / camera->zoom),
	};
}

// Snap a world point to the grid (cell size in pixels)
point_t snap_to_grid(point_t world, int cell_w, int cell_h) {
	if (cell_w == 0) cell_w = 1;
	if (cell_h == 0) cell_h = 1;

	return (point_t){
		.x = (world.x / cell_w) * cell_w,
		.y = (world.y / cell_h) * cell_h,
	};
}

// Converts screen coordinates to tile indices in a layer (rect_in_tiles is in tiles)
point_t cam_screen_to_tile(camera_t *camera, point_t screen, rect_t rect_in_tiles, int cell_w, int cell_h) {
	point_t world = cam_screen_to_world(camera, screen);
	return (point_t){
		.x = (world.x / (rect_in_tiles.w * cell_w)) * rect_in_tiles.w,
		.y = (world.y / (rect_in_tiles.h * cell_h)) * rect_in_tiles.h,
	};
}

// Converts tile coordinates back to screen coordinates (rect_in_tiles is in tiles)
point_t cam_tile_to_screen(camera_t *camera, point_t tile, rect_t rect_in_tiles, int cell_w, int cell_h) {
	point_t world = (point_t){
		.x = ((tile.x * rect_in_tiles.w * cell_w)) / rect_in_tiles.w,
		.y = ((tile.y * rect_in_tiles.h * cell_h)) / rect_in_tiles.h,
	};
	return cam_world_to_screen(camera, world);
}