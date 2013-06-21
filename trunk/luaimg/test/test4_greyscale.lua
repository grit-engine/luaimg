#!../luaimg -F

open("lena.jpg"):map(1, function(c) return (c.x + c.y + c.z)/3 end):save("output_test4.bmp")
