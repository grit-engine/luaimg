#!../luaimg.linux.x86_64 -F 


-- Create a texture basis for a compass widget


-- scaled down to this size to avoid aliasing
local out_sz = vec(94, 94)

-- drawn at this size
local sz = 16 * out_sz

-- create a single 4 pointed rose
simple_rose = make(sz, 3, true, function(pos)

    -- convert pixel position from pixel coords to [-1, 1] range
    pos = pos / (sz - vec(1,1)) * 2 - vec(1, 1)

    -- calculate angle from top, [0, 1) range
    local angle = (deg(atan2(pos.x, pos.y))/360 + 1) % 1

    -- work out what octant we are in
    local oct = floor(angle * 8)

    -- mirror everything to positive X, Y quadrant, then use 2 line equations to work out inside/outside
    pos = abs(pos)
    local inside = 6*pos.x + pos.y < 1 or pos.x + 6*pos.y < 1

    -- compute colour
    local alpha = inside and 1 or 0
    return oct % 2 == 1 and vec(1,1,1,alpha) or vec(0,0,0,alpha)

end)

rotated_rose = simple_rose:rotate(45):scaleBy(0.7, "BOX")

-- scale down to antialias
simple_rose = simple_rose:scale(out_sz, "BOX")
rotated_rose = rotated_rose:scaleBy(out_sz/sz, "BOX")

-- make the actual rose by using the simple rose, with a smaller rotated version behind it
img = simple_rose .. rotated_rose:cropCentre(out_sz, vec(0,0,0,0)) .. make(out_sz, 3, true, vec(0,0,0,0))

img:save("compass_rose.png")
