# WARNING: currently broken do not use

import sys
from PIL import Image

def image_to_font_data(filename):
	c_string = "(sprite_t){\n"
	c_string += "	.data = {\n"

	image = Image.open(filename)
	width, _ = image.size
	pixels = image.load()

	for y in range(16):
		for x in range(16):
			pixel = pixels[x, y]

	c_string += "	.data = },\n"
	c_string += "};"

	return c_string

if __name__ == "__main__":
	print(image_to_font_data(sys.argv[1]))