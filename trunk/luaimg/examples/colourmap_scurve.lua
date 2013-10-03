#!../luaimg -F

local sz = 32

local neutral = make(vector2(sz*sz, sz), 3, function(pos) return vector3(pos.x % sz, sz - pos.y - 1, floor(pos.x / sz)) / (sz-1) end)

function lerp(a,b,c)
    return (1-c)*a + c * b
end

local scurve = neutral:map(3, function(c)
    -- decode gamma
    c = vector3(pow(c.x, 2.2), pow(c.y, 2.2), pow(c.z, 2.2))

    c = c * lerp(vector3(1,1,1), c * (vector3(3,3,3) - 2*c), 1)
    -- encode gamma
    c = vector3(pow(c.x, 1/2.2), pow(c.y, 1/2.2), pow(c.z, 1/2.2))
    return c
end)

scurve:save("output_test11.png")

