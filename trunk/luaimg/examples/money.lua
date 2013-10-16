#!../luaimg -F
img = make(vec(1920,1200),3,0)
--vec4(vec(1,2,3),4)
money = open("money.png")
moneys = {}
local bill_size = money.size / vec(2,4)
for i=0,359 do
    local off = vec(random(2)-1, random(4)-1) * bill_size
    local mask = lerp(vec(0.7,0.7,0.65),vec(1,1,1)*0.8,random())
    moneys[i] = (money:crop(off, bill_size):rotate(i):scaleBy(vec(1,1)/3, "LANCZOS3"))
end
for i=1,10000 do
    local m = moneys[random(359)]
    local pos = vec(random(),random()) * img.size
    img:drawImage(m, pos, true, true)
end

--sharpen
--img = 2*img - img:convolveSep(gaussian(3), true, true)

--img:crop(vec(0,0),vec(2048,2048)):save("moneyimg.png")
img:save("moneyimg.png")
