#!../luaimg.linux.x86_64 -F

-- Given an image and a colour, produce a new image that when blended on top of
-- the colour, produces the original image.  Minimise the alpha channel of the
-- generated image.  This allows the extraction of an alpha mask from a blended
-- image in some circumstances.

args = {...}

if #args ~= 5 then
    print("Usage: <r> <g> <b> <input> <output>")
    return
end

local C = vec(tonumber(args[1]), tonumber(args[2]), tonumber(args[3])) / 255
local input_name = args[4]
local output_name = args[5]

-- avoid divide by zero errors
C = clamp(C, 0.00001*vec(1,1,1), 0.99999*vec(1,1,1))

open(input_name):map(3, true, function(I)
    if I == vec(0,0,0) then
        return vec4(I,1)
    end
    --[[
        I = a * s + (1-a) * C
        minimise a
        0<I<=1
        0<a<=1
        0<s<=1

        I = (1-b)*s + b * C
        maximise b such that
        I - b*C >= 0        i.e.     b <= I/C
        and
        I - b*C + b <= 1    i.e.     b <= (1-I)/(1-C)
        
    ]]
    local tmp1 = I/C
    local tmp2 = (1-I)/(1-C)
    local a = 1 - min(1, tmp1.x, tmp1.y, tmp1.z, tmp2.x, tmp2.y, tmp2.z)
    return vec4(a==0 and vec(0,0,0) or ( I - (1-a)*C ) / a, a)
end):save(output_name)
