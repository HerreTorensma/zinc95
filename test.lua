local x = 0;
local y = 50;

function _init()
	print("Called the init function")
end

function _update()
	x = x + 1
	y = y + 1
end

function _draw()
	cls(2);
	spr(0, x, y, 1, 1)
end