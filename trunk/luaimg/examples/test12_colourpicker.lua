#!../luaimg -F

local picker = make(vector2(256, 256), 3, function(pos) return HSVtoRGB(vector3(0,pos.y/255,pos.x/255)) end)

picker:save("output_test12.png")

