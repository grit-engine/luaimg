#!../luaimg -F

lena = open("lena.jpg")
make(lena.size, 3, function(pos) return lena(pos.x, lena.height - pos.y - 1) end):save("output_test3.jpg")
