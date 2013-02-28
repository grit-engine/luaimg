#!../luaimg -F

open("lena.jpg"):map(function(c) return (c.x + c.y + c.z)/3 end):save("test3.jpg")
