import sys

def hex_to_palette(path):
	c_string = "(palette_t){\n"
	c_string += "	.colors = {\n"

	with open(path, "r") as file:
		for hex in file.readlines():
			r, g, b = tuple(int(hex[i:i+2], 16) for i in (0, 2, 4))
			c_string += f"		{{{r}, {g}, {b}}},\n"

	c_string += "	},\n"
	c_string += "};\n"

	return c_string

if __name__ == "__main__":
	print(hex_to_palette(sys.argv[1]))