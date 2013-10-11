local sz = vec(512,512)
logo = make(sz, 4, true, function(pos)
    local p = pos-sz/2
    if #p > 242 then return 0 end
    if #p > 235 and deg(atan2(p.x, p.y)) * 40/360 % 1 < 0.5 then return 1 end
    if #p > 185 then return 0 end
    return 1
end)

local moon_sz = vec(1,1)*120
local moon = make(moon_sz, 4, true, function(pos)
    local p = (pos/moon_sz-vec(0.5,0.5))*2
    return #p <= 1 and 1 or 0
end)
logo:drawImageAt(0*moon, sz*0.635)
logo:drawImageAt(moon, sz*0.86)

logo:scale(logo.size/5, "LANCZOS3"):save("logo.png")
