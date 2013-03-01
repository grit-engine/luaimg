#!../luaimg -F

img = open("lena.jpg")
make(img.size, 3, function(pos) local c=img(pos) ; return (c.x + c.y + c.z)/3 * vector3(1,1,1) end):save("test3.jpg")
