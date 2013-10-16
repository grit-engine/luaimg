#!../luaimg -F

-- Create a seamless texture that resembles a floor buried in 20 dollar bills

money = open("money.png")

-- precompute an image for every degree of rotation, choosing a random source bill from the money texture
moneys = {}
local bill_size = money.size / vec(2,4)
for i=0,359 do
    -- randomly choose the offset of one of the 8 bills in the source image
    local off = vec(random(2)-1, random(4)-1) * bill_size
    -- crop it out, rotate, and scale
    moneys[i] = money:crop(off, bill_size):rotate(i):scaleBy(vec(1,1)/3, "LANCZOS3")
end

-- allocate the resulting image
img = make(vec(1024,1024),3,0)
for i=1,10000 do
    local m = moneys[random(359)]
    local pos = vec(random(),random()) * img.size
    img:drawImage(m, pos, true, true)
end

--sharpen using unsharp mask technique
img = img + 0.5*(img - img:convolveSep(gaussian(7), true, true))

img:save("moneyimg.png")
