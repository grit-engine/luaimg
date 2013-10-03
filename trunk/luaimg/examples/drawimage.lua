#!../luaimg -F

a = make(vec(256,256),4,true,function(p) return vec(0, 1, 0, p.x/255) end)
b = make(vec(256,256),4,true,function(p) return vec(1, 0, 0, p.y/255) end)
a:drawImage(b,vec(0,0))
a:save("drawimage.png")

lena = open("../test/lena.jpg")
icon_sz = vector2(32,32)
lena_small = lena:scale(icon_sz, "LANCZOS3"):map(4, true, function(v, pos)
    a = #(pos - icon_sz/2)
    a = math.max(0, 2*(icon_sz.x/2 - a)/(icon_sz.x/2))
    return vec4(v.xyz, a)
end)
dst = make(vec(512,512), 3, 0.5)
for y=icon_sz.y/2,dst.height-icon_sz.y,icon_sz.y do
    for x=icon_sz.x/2,dst.width-icon_sz.x,icon_sz.x do
        dst:drawImage(lena_small,vec(x,y))
    end
end
dst:save("drawimage2.png")
