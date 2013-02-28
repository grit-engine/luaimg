#!../luaimg -F

local sz = vector2(1024,1024)

local img = make(sz, 3, function(pos)
    local rel = (pos - sz/2) / (sz/2)
    local length2 = dot(rel,rel)
    if length2 > 1 then return vector3(0,0,0) end
    return vector3(rel.x, rel.y, math.sqrt(1-length2))
end)
img:save('test2.png')
