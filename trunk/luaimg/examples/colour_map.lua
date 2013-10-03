#!../luaimg -F

local sz = 32
make(vec(sz*sz, sz), 3, function(pos)
    return vec(pos.x % sz, sz - pos.y - 1, floor(pos.x / sz)) / (sz - 1)
end):save("colour_map.png")

