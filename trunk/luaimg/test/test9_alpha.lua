#!../luaimg -F

lena = open("lena.jpg")
make(lena.size, 4, function(pos) local v = lena(pos); return vector4(v.x, v.y, v.z, 0.5) end):save("output_test9.png")

