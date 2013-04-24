#!../luaimg -F

local sz = 32
make(vector2(sz*sz, sz), 3, function(pos)
    return vector3(pos.x % sz, sz - pos.y - 1, pos.x / sz) / sz
end):save("output_test10.png")

