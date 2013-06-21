#!../luaimg -F

local lena = open("lena.jpg"):pow(2.2)
local upside_down_lena = make(lena.size, 3, function(pos) return lena(pos.x, lena.height - pos.y - 1) end)
local mix_lena = vector3(3,3,3)*(upside_down_lena + lena)
local mix_lena = mix_lena / (vector3(1,1,1) + mix_lena) -- tone map
mix_lena:pow(1/2.2):save("output_test5.jpg")
