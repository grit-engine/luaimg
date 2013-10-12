#!../luaimg -F
img = make(vec(512,512),3,0)
--vec4(vec(1,2,3),4)
money = open("money.png"):map(4,true,function(c)return vec4(c.x,c.y,c.z,1)end)
moneys = {}
for i=0,359 do
    moneys[i] = lerp(vec(0.7,0.7,0.65),vec(1,1,1)*0.8,random())*money:rotate(i)
end
for i=1,10000 do
    local m = moneys[random(359)]
    local pos = vec(random(),random()) * img.size
    img:drawImage(m, pos, true, true)
end

--sharpen
img = 2*img - img:convolveSep(gaussian(3), true, true)

img:crop(vec(0,0),vec(2048,2048)):save("moneyimg.png")
