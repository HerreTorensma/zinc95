#include "util.h"
#include "../api/api.h"

#include <stdio.h>

void load_palette_from_disk(computer_t *computer, const char filename[]) {
	FILE *file = fopen(filename, "r");
	if (file == NULL) {
		printf("Error opening palette file\n");
	}
	
	int index = 0;
	char line[8];
	while (fgets(line, sizeof(line), file) && index < PALETTE_SIZE) {
		uint32_t packed;
		sscanf(line, "%06X\n", &packed);
		
		rgb_color_t color = {
			.r = (packed & 0xFF0000) >> 16,
			.g = (packed & 0x00FF00) >> 8,
			.b = (packed & 0x0000FF),
		};

		computer->ram->palette.colors[index] = color;
		index++;
	}

	fclose(file);
}

// void draw_out_frame(computer_t *computer, int x, int y, int w, int h) {
// 	// Top white line
// 	api_line(computer, x, y, x + w - 1, y, 15);
// 	// Left white line
// 	api_line(computer, x, y, x, y + h - 1, 15);

// 	// Bottom black line
// 	api_line(computer, x, y + h, x + w, y + h, 0);
// 	// Right black line
// 	api_line(computer, x + w, y, x + w, y + h, 0);

// 	// Bottom gray line
// 	api_line(computer, x + 1, y + h - 1, x + w - 1, y + h - 1, 23);
// 	// Right gray line
// 	api_line(computer, x + w - 1, y + 1, x + w - 1, y + h - 1, 23);
// }

void draw_out_frame(computer_t *computer, int x, int y, int w, int h) {
	// Because we draw lines it will include x + w or y + h in the pixels drawn
	// Which we don't want so subtract 1
	w--;
	h--;

	// Top gray line
	api_line(computer, x, y, x + w - 1, y, 7);
	// Left gray line
	api_line(computer, x, y, x, y + h - 1, 7);

	// Top white line
	api_line(computer, x + 1, y + 1, x + w - 1, y + 1, 15);
	// Left white line
	api_line(computer, x + 1, y + 1, x + 1, y + h - 1, 15);

	// Bottom black line
	api_line(computer, x, y + h, x + w, y + h, 0);
	// Right black line
	api_line(computer, x + w, y, x + w, y + h, 0);

	// Bottom gray line
	api_line(computer, x + 1, y + h - 1, x + w - 1, y + h - 1, 23);
	// Right gray line
	api_line(computer, x + w - 1, y + 1, x + w - 1, y + h - 1, 23);

	// api_rectf(computer, x + 2, y + 2, w - 3, h - 3, 7);
}

void draw_in_frame(computer_t *computer, int x, int y, int w, int h) {
	// Again, because we draw lines it will include x + w or y + h in the pixels drawn
	// Which we don't want so subtract 1
	w--;
	h--;

	// Top gray line
	api_line(computer, x, y, x + w - 1, y, 23);
	// Left gray line
	api_line(computer, x, y, x, y + h - 1, 23);

	// Top black line
	api_line(computer, x + 1, y + 1, x + w - 1, y + 1, 0);
	// Left black line
	api_line(computer, x + 1, y + 1, x + 1, y + h - 1, 0);

	// Bottom white line
	api_line(computer, x, y + h, x + w, y + h, 15);
	// Right white line
	api_line(computer, x + w, y, x + w, y + h, 15);

	// Bottom gray line
	api_line(computer, x + 1, y + h - 1, x + w - 1, y + h - 1, 7);
	// Right gray line
	api_line(computer, x + w - 1, y + 1, x + w - 1, y + h - 1, 7);

	// api_rectf(computer, x + 2, y + 2, w - 3, h - 3, 15);
}