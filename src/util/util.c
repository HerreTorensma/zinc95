#include "util.h"

#include <stdio.h>

void load_palette_from_disk(computer_t *computer, const char filename[]) {
	FILE *file = fopen(filename, "r");
	if (file == NULL) {
		printf("Error opening palette file\n");
	}
	
	int index = 0;
	char line[8];
	while (fgets(line, sizeof(line), file) && index < MAX_COLORS) {
		uint32_t packed;
		sscanf(line, "%06X\n", &packed);
		
		rgb_color_t color = {
			.r = (packed & 0xFF0000) >> 16,
			.g = (packed & 0x00FF00) >> 8,
			.b = (packed & 0x0000FF),
		};

		// printf("r: %d, g: %d, b: %d\n", color.r, color.g, color.b);
		
		computer->ram.palette.colors[index] = color;
		index++;
	}

	fclose(file);
}