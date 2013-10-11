local shadow_colour = vec(1,1,1,1)
local luaimg_colour = vec(0,0,0,1)
local bg_colour = vec(0,0,0,0)

local sz = vec(512,512)
logo = make(sz, 4, true, function(pos)
    local p = pos-sz/2
    local angle = deg(atan2(p.x, p.y))/360
    if #p > 240 then return bg_colour end
    if #p > 235 and angle * 40 % 1 < 0.5 then return luaimg_colour end
    if #p > 185 then return bg_colour end
    return luaimg_colour
end)

local moon_sz = vec(1,1)*120
local moon = make(moon_sz, 4, true, function(pos)
    local p = (pos/moon_sz-vec(0.5,0.5))*2
    return #p <= 1 and 1 or 0
end)
logo:drawImageAt(shadow_colour*moon, sz*0.635)
logo:drawImageAt(luaimg_colour*moon, sz*0.86)

logo:scale(logo.size/5, "LANCZOS3"):save("logo.png")

