#!../luaimg -F

local picker = make(vec(256, 256), 3, function(pos) return HSVtoRGB(vec3(0,pos.yx/255)) end)

picker:save("colour_picker.png")

