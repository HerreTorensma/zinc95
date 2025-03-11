import sys
from PIL import Image


def create_color_dict(palette_path):
	color_dict = {}

	with open(palette_path, "r") as file:
		for idx, line in enumerate(file):
			color = line.strip()
			if color not in color_dict:
				color_dict[color] = idx
				
	return color_dict


def image_to_spritesheet_c_string(image_path, palette_path):
	c_string = "{\n"
	color_dict = create_color_dict(palette_path)

	image = Image.open(image_path)
	width, height = image.size
	pixels = image.load()

	for y in range(height):
		c_string += "	"
		
		for x in range(width):
			rgb_color = pixels[x, y]
			hex_color = f"{rgb_color[0]:02x}{rgb_color[1]:02x}{rgb_color[2]:02x}"
			palette_color = color_dict[hex_color]
			c_string += f"0x{palette_color:02x}, "

		c_string += "\n"

	c_string += "}\n"

	return c_string


if __name__ ==  "__main__":
	print(image_to_spritesheet_c_string(sys.argv[1], sys.argv[2]))