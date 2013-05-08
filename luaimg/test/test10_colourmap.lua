#!../luaimg -F

local sz = 32
make(vector2(sz*sz, sz), 3, function(pos)
    return vector3(pos.x % sz, sz - pos.y - 1, floor(pos.x / sz)) / (sz - 1)
end):save("output_test10.png")

