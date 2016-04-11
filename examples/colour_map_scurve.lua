#!../luaimg.linux.x86_64 -F

-- Create a colour grade LUT that brings out more contrast in the midtones.
-- This grading used to be hardcoded into the Grit shaders before I added arbitrary LUT support.

local sz = 32

local neutral = make(vec(sz*sz, sz), 3, function(pos) return vec(pos.x % sz, sz - pos.y - 1, floor(pos.x / sz)) / (sz-1) end)

local scurve = neutral:map(3, function(c)
    -- decode gamma
    c = c ^ 2.2

    c = c *  c * (3 - 2*c)

    -- encode gamma
    c = c ^ (1/2.2)

    return c
end)

scurve:save("colour_map_scurve.png")

