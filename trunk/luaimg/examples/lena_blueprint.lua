#!../luaimg.linux.x86_64 -F

-- Create a lena "blueprint" image (just a bit of fun)

-- greyscale Lena with a little gaussian blur to fix noise, etc
lena = open(`lena_std.png`)
lena = ((lena.x + lena.y + lena.z) / 3):convolveSep(gaussian(9))

-- simple edge detection with a laplace kernel
laplace = lena:scale(vec(256,256),"LANCZOS3"):convolve(make(vec(3,3),1,{0,-1,0, -1,4,-1, 0,-1,0}):normalise())
-- renormalise by calculating range
M = laplace:reduce(0, function(a,b) return max(a,b) end)
m = -(-laplace):reduce(0, function(a,b) return max(a,b) end)
laplace = ((laplace - m)/(M-m))

local company_colour = vec(48,80,112)/255

img = laplace:map(3, function (n)
    n = n * n * (3 - 2*n)
    if n < 0.45 then
        n = n / 0.45
        return lerp(vec(0,0,0), company_colour, n)
    elseif n < 0.55 then
        return company_colour
    else
        n = (n - 0.55) / 0.45
        return lerp(company_colour, vec(1,1,1), n)
    end
end)

-- now, round off the corners so nobody can accidently cut themselves
local corner_rad = 8
img = img:map(3,true,function(c,p)
    -- map to bottom left quadrant
    local x, y = p.x, p.y
    if x > img.width/2 then x = img.width - x - 1 end
    if y > img.height/2 then y = img.height - y - 1 end
    x = (corner_rad - x)/corner_rad
    y = (corner_rad - y)/corner_rad
    if x<0 or y<0 then return vec4(c,1) end
    if x*x + y*y<1 then return vec4(c,1) else return vec(0,0,0,0) end
end)
img:save("lena_blueprint.png")
