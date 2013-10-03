#!../luaimg -F

local sz = 32

local neutral = make(vec(sz*sz, sz), 3, function(pos) return vec(pos.x % sz, sz - pos.y - 1, floor(pos.x / sz)) / (sz-1) end)

local scurve = neutral:map(3, function(c)
    -- decode gamma
    c = c ^ 2.2

    c = c *  c * (vec(3,3,3) - vec(2,2,2)*c)

    -- encode gamma
    c = c ^ (1/2.2)

    return c
end)

scurve:save("output_test11.png")

