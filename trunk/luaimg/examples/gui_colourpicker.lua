#!../luaimg -F

-- A GUI texture used by Grit.  Basically a scale for picking saturation and luminance.

local picker = make(vec(256, 256), 3, function(pos) return HSVtoRGB(vec3(0,pos.yx/255)) end)

picker:save("colour_picker.png")

