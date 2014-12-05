#!../luaimg.linux.x86_64 -F

img = open("../examples/lena_std.png"):scale(vec(256, 256), "BOX")

tab = {}
for i=1,100 do
    tab[i] = img:quantise("FLOYD_STEINBERG", vec(4,8,8))
    img = img:convolveSep(gaussian(5), false, false)
end

for i=101,200 do
    tab[i] = tab[200 - i + 1]
end

gif_save("aniblur.gif", 0, tab, 0.02)
