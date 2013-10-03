#!../luaimg -F

a = make(vector2(256,256),4,true,function(p) return vector4(0, 1, 0, p.x/255) end)
b = make(vector2(256,256),4,true,function(p) return vector4(1, 0, 0, p.y/255) end)
a:drawImage(b,vector2(0,0))
a:save("output_test14_drawimage.png")

lena = open("lena.jpg")
icon_sz = vector2(32,32)
lena_small = lena:scale(icon_sz, "LANCZOS3"):map(4, true, function(v, pos)
    a = #(pos - icon_sz/2)
    a = math.max(0, 2*(icon_sz.x/2 - a)/(icon_sz.x/2))
    return vector4(v.x, v.y, v.z, a)
end)
dst = make(vector2(512,512),3,function(p) return vector3(0.5, 0.5, 0.5) end)
for y=icon_sz.y/2,dst.height-icon_sz.y,icon_sz.y do
    for x=icon_sz.x/2,dst.width-icon_sz.x,icon_sz.x do
        dst:drawImage(lena_small,vector2(x,y))
    end
end
dst:save("output_test14_drawimage2.png")
