#!../luaimg -F

open("lena.jpg"):map(1, function(c) return vector3(1,1,1) * (c.x + c.y + c.z)/3 end):save("output_test4.bmp")
