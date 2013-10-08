errors = {}
num_success = 0
function append_error(str)
    errors[#errors+1] = str
end
function print_errors()
    local num_errors = #errors
    if num_errors > 0 then
        print("FAIL: "..num_errors.." / "..(num_errors+num_success).." did not pass... :(")
        for k,v in ipairs(errors) do
            print(v)
        end
    else
        print("SUCCESS: "..num_success.." / "..(num_errors+num_success).." passed! :)")
    end
end

function require_eq(name, a,b)
    if a ~= b then
        append_error(name..": Expected these to be equal: "..tostring(a).." and "..tostring(b))
    else
        num_success = num_success + 1
    end
end

function require_mean(name, a, b, thresh)
    local v = a:meanDiff(b)
    thresh = thresh or 0
    local len = type(v) == "number" and v or #v
    if len > thresh then
        append_error(name..": Failed RMS test with #"..v.." = "..len.." (threshold was "..thresh..")")
    else
        num_success = num_success + 1
    end
end

function require_rms(name, a, b, thresh)
    local v = a:rmsDiff(b)
    thresh = thresh or 0
    local len = type(v) == "number" and v or #v
    if len > thresh then
        append_error(name..": Failed RMS test with #"..v.." = "..len.." (threshold was "..thresh..")")
    else
        num_success = num_success + 1
    end
end

function require_img_eq_val(name, img, val)
    local bad = 0
    for y=0,img.height-1 do
        for x=0,img.width-1 do
            if img(x,y) ~= val then
                bad = bad + 1
            end
        end
    end
    if bad ~= 0 then
        append_error(name..": images did not match solid value")
    else
        num_success = num_success + 1
    end
end
function require_img_eq_func(name, img, func)
    local bad = 0
    for y=0,img.height-1 do
        for x=0,img.width-1 do
            local val = func(vec(x,y))
            if img(x,y) ~= val then
                bad = bad + 1
            end
        end
    end
    if bad ~= 0 then
        append_error(name..": images did not match function")
    else
        num_success = num_success + 1
    end
end


-- MAKE TESTS

for _,val in ipairs{0.5, vec(1,2), vec(1,2,3),vec(1,2,3,4)} do
    local d = val.dim
    local img1 = make(vec(40,30),d,val)
    require_img_eq_val("make-val-"..d, img1, val)
    local img1b = make(vec(40,30),d,function()return val end)
    require_img_eq_val("make-map-"..d, img1b, val)
    require_mean("make-val-"..d.."-mean0", img1-img1, 0)
    require_mean("make-val-"..d.."-mean", img1, val)
    require_rms("make-val-"..d.."-rms0", img1-img1, 0)
    require_rms("make-val-"..d.."-rms", img1, val)
    require_rms("make-map-"..d.."-rms0", img1b-img1b, 0)
    require_rms("make-map-"..d.."-rms", img1b, val)
    require_rms("make-both-"..d.."-rms", img1, img1b)

    local img2 = make(vec(40,30),d,true,val)
    local img2b = make(vec(40,30),d,true,function()return val end)
    require_img_eq_val("make-val-alpha-"..d, img2, val)
    require_img_eq_val("make-map-alpha-"..d, img2b, val)
    require_rms("make-val-alpha"..d.."-rms", img2, val)
    require_rms("make-map-alpha"..d.."-rms", img2b, val)
    require_rms("make-both-alpha"..d.."-rms", img2, img2b)
end

imgbase_init = function(p)return vec3(p,0)/39 end
imgbase = make(vec(40,30),3,imgbase_init)
require_img_eq_func("make-closure", imgbase, imgbase_init)
require_rms("make-closure-self-rms", imgbase, imgbase)

imgbase_a_init = function(p)return vec4(p/39,0,0.5) end
imgbase_a = make(vec(40,30),4,true,imgbase_a_init)
require_img_eq_func("make-closure-alpha", imgbase_a, imgbase_a_init)
require_rms("make-closure-alpha-self-rms", imgbase_a, imgbase_a)


function try_io(img, ext, thresh)
    local filename = os.tmpname()..ext
    img:save(filename)
    require_rms("png-io-"..filename,open(filename),img, thresh)
    --os.remove(filename)
end

try_io(imgbase, ".png", 1/255)
try_io(imgbase, ".jpg", 1/40)

--local imgbase_a_pma = imgbase_a:map(4,true,function(c)return vec4(c.xyz * c.w, c.w)end)
try_io(imgbase_a, ".png", 1/255)


lena = open("lena.jpg")
lena_a = lena:map(4,true,function(c)return vec4(c,0.5)end)

-- SIMPLE TRANSFORMATIONS AND MAP
function simpletrans(name,img)
    require_rms(name.."-map-identity-rms", img, img:map(img.channels, img.hasAlpha, function(col, pos) return col end))
    require_rms(name.."-map-identity2-rms", img, img:map(img.channels, img.hasAlpha, function(col, pos) return img(pos) end))
    require_rms(name.."-map-mirror-rms", img:mirror(), img:map(img.channels, img.hasAlpha, function(col, pos) return img(pos*vec(-1,1)+vec(img.width-1,0)) end))
    require_rms(name.."-map-flip-rms", img:flip(), img:map(img.channels, img.hasAlpha, function(col, pos) return img(pos*vec(1,-1)+vec(0,img.height-1)) end))
    require_rms(name.."-map-rotate180-rms", img:flip():mirror(), img:rotate(180))
    local scaled = make(img.size*2, img.channels, img.hasAlpha, function(p) return img(p/2) end)
    require_rms(name.."-map-scale-rms", img:scale(img.size*2,"LANCZOS3"), scaled,0.05)
end

require_rms("gaussian", gaussian(10), make(vec(10,1), 1, {1,9,36,84,126,126,84,36,9,1}):normalise())
simpletrans("lena",lena)
simpletrans("lena-a",lena_a)
simpletrans("lena.x",lena.x)
simpletrans("imgbase",imgbase)

--ARITHMETIC
require_rms("add-rms", imgbase+1, vec(1,1,1)+imgbase)
require_rms("sub-rms0", imgbase-imgbase, 0)
require_rms("unm-rms0", -imgbase + imgbase, 0)
require_rms("abs-pos-rms", imgbase, imgbase:abs())
require_rms("abs-neg-rms", imgbase, (-imgbase):abs())
require_rms("mul-rms", imgbase * 2, imgbase+imgbase)
require_rms("mul2-rms", imgbase * 2, 2 * imgbase)
require_rms("div-rms", (imgbase * vec(2,2,2)) / 2, imgbase)
require_rms("pow-rms", imgbase * imgbase, imgbase ^ 2)
require_rms("pow2-rms", (imgbase ^ 2.2) ^ (1/2.2), imgbase, 1e-5)
require_rms("max-rms", imgbase:max(imgbase*2), imgbase*2)
require_rms("min-rms", imgbase:min(imgbase*2), imgbase)

require_rms("map-add-rms", 3.5+imgbase, imgbase:map(3, function(col) return colour(3,3.5)+col end))
require_rms("map-mul-rms", 3.5*imgbase, imgbase:map(3, function(col) return 3.5*col end))
require_rms("map-div-rms", 1/(imgbase+1), imgbase:map(3, function(col) return vec(1,1,1)/(col+vec(1,1,1)) end))

require_rms("grey-rms", (imgbase.x + imgbase.y + imgbase.z)/3, imgbase:map(3, function(col) return (col.x + col.y + col.z)/3 end), 1e-7)

-- REDUCE
local lena_max = vec(0,0,0)
for y=0,lena.height-1 do
    for x=0,lena.width-1 do
        local a = lena(x,y)
        lena_max = vec(max(a.x, lena_max.x), max(a.y, lena_max.y), max(a.z,lena_max.z))
    end
end
local lena_max2 = lena:reduce(vec(0,0,0), function(a,b) return vec(max(a.x, b.x), max(a.y, b.y), max(a.z,b.z)) end)
local lena_max3 = vec(0,0,0)
lena:foreach(function(a) local b=lena_max3 ; lena_max3 = vec(max(a.x, b.x), max(a.y, b.y), max(a.z,b.z)) end)
require_eq("reduce", lena_max, lena_max2)
require_eq("foreach", lena_max, lena_max3)

-- SET
img1 = make(vec(2,2), 3, 1)
img2 = make(vec(2,2), 3, 2)
img1:set(vec(0,0), 2)
img1:set(vec(0,1), 2)
img1:set(vec(1,0), 2)
img1:set(vec(1,1), vec(2,2,2))
require_rms("set", img1, img2)

imgn = make(vec(2,2), 3, 0.25)
require_rms("norm1", imgn, img1:normalise())
require_rms("norm2", imgn, img2:normalise())

require_rms("swizzle-yz", lena.yz, lena:map(2,function(c)return c.yz end))
require_rms("swizzle-zx", lena.zx, lena:map(2,function(c)return vec(c.z,c.x) end))
require_rms("swizzle-xZ", lena.xZ, lena:map(2,true,function(c)return vec(c.x,c.z) end))

require_rms("gaussian", gaussian(10), make(vec(10,1), 1, {1,9,36,84,126,126,84,36,9,1}):normalise())

init = function(p) return lena_a(p).xyz * lena_a(p).w + lena(p*vec(-1,1)+vec(lena.width-1,0)) end
require_rms("add-alpha", lena_a + lena:mirror(), make(lena.size, 3, init), 1e-7)

init = function(p) return lena_a(p).w * lena_a(p).xyz + (1-lena_a(p).w) * lena(p*vec(-1,1)+vec(lena.width-1,0)) end
require_rms("blend-alpha", lena_a .. lena:mirror(), make(lena.size, 3, init), 1e-7)

require_rms("blend-alpha2", make(vec(1,1),2,true,vec(0.3,0.4))..make(vec(1,1),1,vec(0.7)), lerp(0.7,0.3,0.4))
require_rms("blend-alpha3", vec(1,1,1,0.25)..make(vec(1,1),3,0.2), make(vec(1,1),3,0.4))
require_rms("blend-alpha3", vec(1,1,1)..make(vec(1,1),3,0.2), make(vec(1,1),3,1))
require_eq("lerp", lerp(10,20,0.4), 14)

require_rms("crop", lena:crop(vec(10,10),vec(100,100),0), make(vec(100,100),3,function(p) return lena(p+vec(10,10)) end))
require_rms("crop2", lena:crop(vec(-10,-10),vec(800,800),vec(0.5,0.5,0.2)), make(vec(800,800),3,function(p)
    p = p + vec(-10,-10)
    local bg = vec(0.5,0.5,0.2)
    if p.x >= lena.width then return bg end
    if p.y >= lena.height then return bg end
    if p.x < 0 then return bg end
    if p.y < 0 then return bg end
    return lena(p)
end))

kernel = make(vec(5,5), 1, { 0,1,5,3,2, 0,1,6,2,3, 4,7,1,0,0, 2,5,4,4,1, 1,1,2,1,1, }):normalise()
img = make(vec(5,5), 1, 0)
img:set(vec(2,2), 2);
kernel_match = img:convolve(kernel):flip():mirror()/2
require_rms("convolve", kernel_match, kernel)

kernel3 = make(vec(5,1), 1, { 0,1,1,1,0 }):normalise()
kernel2 = make(vec(5,5), 1, { 0,0,0,0,0, 0,1,1,1,0, 0,1,1,1,0, 0,1,1,1,0, 0,0,0,0,0, }):normalise()
img2 = make(vec(5,5), 1, 0)
img2:set(vec(2,2), 2);
require_rms("convolvesep", img2:convolveSep(kernel3):flip():mirror()/2, kernel2,1e-8)

print_errors()
