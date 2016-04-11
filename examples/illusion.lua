sz = vec(600, 600)
scale = 4

sqsz = 18 * scale
spacing = 25 * scale
side = 2.25 * scale

square = make((sqsz - 2*side)*vec(1,1), 1, 0):cropCentre(sqsz*vec(1,1), 1)

white = square.fffX
black = square.eeeX

bg = vec(77,128,180)/255

function draw_ring(img, rad, offset)
    local circ = 2 * rad * pi
    local num = floor(circ / spacing / 2) * 2
    for i=0, num-1 do
        local angle = i / num * pi * 2
        local sq = i % 2 == 0 and black or white
        local pos = vec(sin(angle), cos(angle)) * rad
        img:drawImageAt(sq:rotate(angle / pi * 180 + offset), pos + sz*scale/2)
    end
end

tab = {}
frames = 100
for i=0, frames-1 do
    print("Drawing frame " .. i)
    local img = make(scale*sz, 3, bg)
    local angle = 20 * sin(i / frames / pi * 180)
    draw_ring(img, 255 * scale, angle)
    draw_ring(img, 196 * scale, angle)
    draw_ring(img, 135 * scale, angle)
    draw_ring(img, 75 * scale, angle)
    img = img:scale(vec(256, 256), "BOX")
    tab[i] = img + (img - img:convolve(gaussian(3)))
end

gif_save("illusion.gif", 0, tab, 0.05)
