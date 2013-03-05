#!../luaimg -F

local lena = open("lena.jpg")
local lena_max = lena:reduce(vector3(0,0,0), function(a,b) return vector3(max(a.x, b.x), max(a.y, b.y), max(a.z,b.z)) end)
lena_max = max(lena_max.x, lena_max.y, lena_max.z) * vector3(1,1,1);
(lena / lena_max):save("output_test7.jpg")
