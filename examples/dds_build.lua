#!../luaimg.linux.x86_64 -F

if select('#', ...) ~= 2 then
    error("Usage: luaimg -F dds_build.lua <in.png> <out.dds>")
end
local in_file = select(1, ...)
local out_file = select(2, ...)


function gen_mipmaps(img)
    local tab = {}
    local counter = 1
    local w = img.width
    local h = img.height
    while w>1 or h>1 do
        tab[counter] = img:scale(vec(w, h), "BOX")
        w = w == 1 and 1 or ceil(w/2)
        h = h == 1 and 1 or ceil(h/2)
        counter = counter + 1
    end
    return tab
end

img = open(in_file)

if not img.hasAlpha then img = img.xyzF end

fmt = "BC1"

dds_save_simple(out_file, fmt, gen_mipmaps(img))
