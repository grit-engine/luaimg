#!../luaimg -F

local sz = vector2(1024,1024)

local img = make(sz, 3, function(pos)
    local hsz = sz/2;
    local rel = (pos - hsz) / hsz
    local length2 = dot(rel,rel)
    if length2 >= 1 then return vector3(0,0,0) end
    return vector3(1+rel.x, 1+rel.y, 2-math.sqrt(length2))/2
end)
img:save('output_test2.png')
