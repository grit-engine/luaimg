local d = 0.2
local a = 4 * 513.5

local out_sz = 256 -- 600

function make_canvas(bw_offset)
    img = make(vec(1200, 1200), 3, function(p)
        local r = #p
        local angle = atan2(p.y, p.x) / pi * 2
        for i = 0, 14 do
            if r > a * exp(-d * 2*i) then
                if floor(bw_offset + angle * 8) % 2 == i % 2 then
                    return 1
                else
                    return 0
                end
            elseif r > a * exp(-0.2 * (2*i+1)) then
                if floor(angle * 8) % 2 == i % 2 then
                    return vec(0, 209, 255) / 255
                else
                    return vec(0, 153, 255) / 255
                end
            end
        end
        return vec(0, 209, 255) / 255
    end)

    local checker = make(vec(2,2), 1, {1, 0, 0, 1}):scale(vec(100, 100), "BOX").xxxF

    for i = 0, 14 do
        local middle = a * exp(-d * 2*i)
        local inner = a * exp(-d * (2*i + 1))
        local off = (i % 2 == 0) and 90 or 0
        for j = -2, 8 do
            local angle = ((j + bw_offset)/8 * 90)
            local ang = angle / 180 * pi
            local off2 = off + ((j % 2 == 0) and 90 or 0)
            local brush = checker:scaleBy(inner/1250, "LANCZOS3"):rotate(angle + 45 + off2)
            img:drawImageAt(brush, inner * vec(sin(ang), cos(ang)))
            brush = checker:scaleBy(middle/1250, "LANCZOS3"):rotate(angle + 45 + off2)
            img:drawImageAt(brush, middle * vec(sin(ang), cos(ang)))
        end
    end

    img = img:scale(vec(out_sz, out_sz) / 2, "BOX").xyzF

    canvas = make(vec(out_sz, out_sz), 3, 0)
    canvas:drawImage(img:rotate(0), vec(out_sz, out_sz)/2)
    canvas:drawImage(img:rotate(90), vec(out_sz/2, 0))
    canvas:drawImage(img:rotate(180), vec(0, 0))
    canvas:drawImage(img:rotate(270), vec(0, out_sz/2))

    canvas = canvas + (canvas - canvas:convolve(gaussian(3)))
    return canvas

end

tab = { }
local num_frames = 30
for i=0, num_frames - 1 do
    tab[i+1] = make_canvas(i / num_frames * 2):quantise("NONE", vec(2, 256, 2))
end
gif_save("illusion2.gif", 0, tab, 1 / num_frames)
