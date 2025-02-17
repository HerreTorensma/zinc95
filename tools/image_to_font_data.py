import sys
from PIL import Image

def image_to_font_data(filename, char_width, char_height):
	c_string = "(font_data_t){\n"
	c_string += "	.sprites = {\n"

	image = Image.open(filename)
	width, _ = image.size
	pixels = image.load()

	for sprite_index in range(width // char_width):
		c_string += "		(sprite_t){\n"
		c_string += "			.data = {\n"

		for y in range(char_height):
			c_string += "				"
			for x in range(char_width):
				pixel = pixels[(sprite_index * char_width) + x, y]

				if pixel == (0, 0, 0, 255):
					c_string += "0x00, "
				elif pixel == (255, 255, 255, 255):
					c_string += "0x0F, "

			c_string += "\n"

		c_string += "			},\n"
		c_string += "		},\n"

	c_string += "	},\n"
	c_string += "};\n"

	return c_string

if __name__ == "__main__":
	print(image_to_font_data(sys.argv[1], int(sys.argv[2]), int(sys.argv[3])))