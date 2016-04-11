#!../luaimg.linux.x86_64 -F

-- Create a seamless texture that resembles a floor buried in 20 dollar bills.

-- Do all operations in linear space, for greater fidelity
local gamma = 2.2

money = open("../examples/money_input.png"):gamma(gamma)

-- precompute an image for every degree of rotation, choosing a random source bill from the money
-- texture
moneys = {}
local bill_size = money.size / vec(2,4)
for i=0,359 do
    -- randomly choose the offset of one of the 8 bills in the source image
    local off = vec(random(2)-1, random(4)-1) * bill_size
    -- crop it out, rotate, and scale
    moneys[i] = money:crop(off, bill_size):rotate(i):scaleBy(1/3, "LANCZOS3")
end

-- Calculate where all the bills will go
img = make(vec(512,512), 3, 0)
angles, positions = {}, {}
iters = 200
for i=1,iters do
    angles[i] = random(359)
    positions[i] = vec(random(),random()) * img.size
end

-- Draw an initial version of the image (all bills)
for i=1,iters do
    img:drawImage(moneys[angles[i]], positions[i], true, true)
end

-- Draw all the bills again, this time cacheing each intermediate image
tab = {}
for i=1,iters/4 do
    img:drawImage(moneys[angles[4 * i - 3]], positions[4 * i - 3], true, true)
    img:drawImage(moneys[angles[4 * i - 2]], positions[4 * i - 2], true, true)
    img:drawImage(moneys[angles[4 * i - 1]], positions[4 * i - 1], true, true)
    img:drawImage(moneys[angles[4 * i - 0]], positions[4 * i - 0], true, true)
    --sharpen using unsharp mask method
    local sharpened = (img + 0.5*(img - img:convolveSep(gaussian(5), true, true)))
    -- scale down and gamma encode output
    tab[i] = sharpened:scale(vec(256,256), "BOX"):gamma(1/gamma)
end

-- save as anigif
gif_save("money.gif", 0, tab, 0.05)
