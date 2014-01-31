#!../luaimg.linux.x86_64 -F 

sz = vec(512, 512)

function xyz_to_colour(pos)
        pos = norm(pos)
        local tpos = quat(37, norm(vec(0.15, 0.45, 0.38))) * pos
        local az = deg(atan2(tpos.x, tpos.y))
        local el = deg(acos(tpos.z))
        local closest_az = floor(az/18+0.5)*18
        local col = vec(0,0,0)
        if abs(az - closest_az) < 1 then
                col = col + (1-min(1,abs(az - closest_az) / 1)) * vec(1,0,0)
        end
        local closest_el = floor(el/18+0.5)*18
        if abs(el - closest_el) < 0.5 then
                col = col + (1-min(1,abs(el - closest_el) / 0.5)) * vec(0,1,0)
        end
        if dot(pos, vec(1,0,0)) > 0.9 then col = col + vec(1,0,0) end
        if dot(pos, vec(0,1,0)) > 0.9 then col = col + vec(0,1,0) end
        if dot(pos, vec(0,0,1)) > 0.9 then col = col + vec(0,0,1) end
        if dot(pos, vec(-1,0,0)) > 0.9 then col = col + vec(0,1,1) end
        if dot(pos, vec(0,-1,0)) > 0.9 then col = col + vec(1,0,1) end
        if dot(pos, vec(0,0,-1)) > 0.9 then col = col + vec(1,1,0) end
        return vec4(col,1)
end

function norm_texel(pos)
        return (pos-(sz-1)/2) / (sz/2)
end

X = make(sz, 3, true, function (pos) local p = norm_texel(pos) ; return xyz_to_colour(vec( 1,  -p.y, -p.x)) end)
x = make(sz, 3, true, function (pos) local p = norm_texel(pos) ; return xyz_to_colour(vec(-1,  -p.y,  p.x)) end)
Y = make(sz, 3, true, function (pos) local p = norm_texel(pos) ; return xyz_to_colour(vec(-p.x,  1,  p.y)) end)
y = make(sz, 3, true, function (pos) local p = norm_texel(pos) ; return xyz_to_colour(vec(-p.x, -1, -p.y)) end)
Z = make(sz, 3, true, function (pos) local p = norm_texel(pos) ; return xyz_to_colour(vec( p.x, -p.y,  1)) end)
z = make(sz, 3, true, function (pos) local p = norm_texel(pos) ; return xyz_to_colour(vec(-p.x, -p.y, -1)) end)
--[[
X = make(sz, 3, true, function (pos) local p = norm_texel(pos) ; return xyz_to_colour(vec( 1, -p.y, -p.x)) end)
x = make(sz, 3, true, function (pos) local p = norm_texel(pos) ; return xyz_to_colour(vec(-1, -p.y,  p.x)) end)
Y = make(sz, 3, true, function (pos) local p = norm_texel(pos) ; return xyz_to_colour(vec( p.x,  1,  p.y)) end)
y = make(sz, 3, true, function (pos) local p = norm_texel(pos) ; return xyz_to_colour(vec( p.x, -1, -p.y)) end)
Z = make(sz, 3, true, function (pos) local p = norm_texel(pos) ; return xyz_to_colour(vec( p.x, -p.y,  1)) end)
z = make(sz, 3, true, function (pos) local p = norm_texel(pos) ; return xyz_to_colour(vec(-p.x, -p.y, -1)) end)
]]

X:drawImageAt(text("/usr/share/fonts/truetype/msttcorefonts/Impact.ttf", vec(100,100), "+x").fffX, sz/2)
x:drawImageAt(text("/usr/share/fonts/truetype/msttcorefonts/Impact.ttf", vec(100,100), "-x").fffX, sz/2)
Y:drawImageAt(text("/usr/share/fonts/truetype/msttcorefonts/Impact.ttf", vec(100,100), "+y").fffX, sz/2)
y:drawImageAt(text("/usr/share/fonts/truetype/msttcorefonts/Impact.ttf", vec(100,100), "-y").fffX, sz/2)
Z:drawImageAt(text("/usr/share/fonts/truetype/msttcorefonts/Impact.ttf", vec(100,100), "+z").fffX, sz/2)
z:drawImageAt(text("/usr/share/fonts/truetype/msttcorefonts/Impact.ttf", vec(100,100), "-z").fffX, sz/2)


X:save("X.png")
x:save("x.png")
Y:save("Y.png")
y:save("y.png")
Z:save("Z.png")
z:save("z.png")

dds_save_cube("cube.dds", "BC1", mipmaps(X,"BOX"), mipmaps(x,"BOX"), mipmaps(Y,"BOX"), mipmaps(y,"BOX"), mipmaps(Z,"BOX"), mipmaps(z,"BOX"))
