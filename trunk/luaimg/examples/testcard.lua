matrix = {
    vec(0, 0, 0, 63),
    vec(0, 0, 63, 63),
    vec(0, 63, 63, 63),
    vec(0, 0, 0, 127),
    vec(63, 63, 63, 127),
    vec(0, 0, 127, 127),
    vec(63, 63, 127, 127),
    vec(0, 0, 0, 191),
    vec(0, 127, 127, 127),
    vec(63, 63, 63, 191),
    vec(63, 127, 127, 127),
    vec(0, 0, 0, 255),
    vec(0, 0, 191, 191),
    vec(63, 63, 63, 255),
    vec(63, 63, 191, 191),
    vec(127, 127, 127, 191),
    vec(127, 127, 191, 191),
    vec(0, 191, 191, 191),
    vec(63, 191, 191, 191),
    vec(127, 127, 127, 255),
    vec(127, 191, 191, 191),
    vec(0, 0, 255, 255),
    vec(63, 63, 255, 255),
    vec(127, 127, 255, 255),
    vec(191, 191, 191, 255),
    vec(0, 255, 255, 255),
    vec(63, 255, 255, 255),
    vec(191, 191, 255, 255),
    vec(127, 255, 255, 255),
    vec(191, 255, 255, 255),
}

local gamma = 2.2

--[[
function blend(c)
    return (c.x + c.y + c.z + c.w) / 4
end
]]
function blend(c)
    local gc = vec4(pow(c.x, gamma),
                    pow(c.y, gamma),
                    pow(c.z, gamma),
                    pow(c.w, gamma))
    return pow((gc.x + gc.y + gc.z + gc.w) / 4, 1/gamma)
end

grid = vec(6, 5)    
cell_sz = vec(80, 80)

img = make(grid * cell_sz, 1, 0)

function pattern(c)
    return make(cell_sz, 1, true, function(p)
        local r = 2 * #(p / cell_sz - 0.5)
        local a = r > 0.8 and 0 or 1
        if p.x % 2 == 0 then
            if p.y % 2 == 0 then
                return vec(c.x, a)
            else
                return vec(c.z, a)
            end
        else
            if p.y % 2 == 0 then
                return vec(c.w, a)
            else
                return vec(c.y, a)
            end
        end
    end)
end

for gy = 0, grid.y-1 do
    for gx = 0, grid.x-1 do
        local c = matrix[gy * grid.x + gx + 1] / 255
        local cell = make(cell_sz, 1, true, vec2(blend(c), 1))
        local circle = pattern(c)
        img:drawImage(cell, vec(gx, gy) * cell_sz)
        img:drawImage(circle, vec(gx, gy) * cell_sz)
    end
end

all_img = make(grid * cell_sz * 2, 3, 0)
all_img:drawImage(img.xxxF, vec(0, 0) * img.size)
all_img:drawImage(img.xeeF, vec(1, 0) * img.size)
all_img:drawImage(img.exeF, vec(0, 1) * img.size)
all_img:drawImage(img.eexF, vec(1, 1) * img.size)
all_img:save("testcard.png")
