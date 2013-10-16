#!../luaimg

local sz = vec(128,128)
local function inside_rect(p, b)
    return abs(p.x) <= b.x and abs(p.y) <= b.y
end
local d0,d1,d2 = vec(40,30),12,15
logo = make(sz, 4, true, function(pos)
    local p = pos-sz/2
    if inside_rect(p, d0) then return 1 end
    if inside_rect(p, vec(d1,d1)+d0) then return 0 end
    if inside_rect(p, vec(d2,d2)+d0) and ((p.x/10+0.25) % 1 >= .5) ~= ((p.y/10+0.25) % 1 >= .5) then return 1 end
    return 0
end)

local moon = make(vec(16,16), 4, true, 1)
logo:drawImageAt(0*moon, vec(88,78))
logo:drawImageAt(moon, vec(117,107))

logo = logo .. vec(48,80,112)/255

logo:scale(logo.size/2, "BOX"):save("logo_med.png")
logo:scale(logo.size/2, "BOX"):crop(vec(32,32),vec(32,32)):save("logo_small.png")
logo:scale(logo.size/2, "BOX"):crop(vec(32,32),vec(32,32)):scale(vec(16,16),"BOX"):save("logo_tiny.png")
logo:save("logo_large.png")
