#!../luaimg.linux.x86_64 -F

local sz = vec(1024,1024)
test_image = make(sz, 3, true, function (p)
        local p2 = p/sz
        local col = vec(1-p2.x^2, p2.y^1.5, p2.x*p2.y)
        local cp = (p2 - 0.5)/0.5
        local alpha = 1 - (1 - dot(cp, cp)) ^ 3
        return vec4(col, alpha)
end)

test_image:save("test_image.png")
