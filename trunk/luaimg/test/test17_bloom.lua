#!../luaimg -F

function tone_map (c)
    local hsv = RGBtoHSV(c)
    local v = hsv.z / (1+hsv.z)
    return HSVtoRGB(vector3(hsv.x, hsv.y, v))
end
function inverse_tone_map (c)
    local hsv = RGBtoHSV(c)
    local v = hsv.z / (1.001-hsv.z)
    return HSVtoRGB(vector3(hsv.x, hsv.y, v))
end

lena = open("lena.jpg"):pow(2.2)

sep_gaussian_x = make(vector2(9,1),1,{ 1, 8, 28, 56, 70, 56, 28, 8, 1 }):normalise()
sep_gaussian_y = sep_gaussian_x:rotate(90);

blurred = lena:convolve(sep_gaussian_x, false, false):convolve(sep_gaussian_y, false, false);

(lena+blurred):pow(1/2.2):save("test17_bloom.png")


-- a = c/(1+c)
-- a = c (1 - a)
-- a/(1-a) = c
