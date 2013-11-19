#!../luaimg.linux.x86_64 -F

-- A script for generating mandelbrot images.

mb = nil

function mb_generate (w, iters)
    w = w or 8192
    iters = iters or 500
    local sz = vec(w, w * 1.5/4)
    print("Generating mandelbrot set of size: "..sz)
    -- 1 channel texture, no alpha channel
    mb = make(sz, 1, function(pos)
        local c = vec(lerp(-2.5,1.5,pos.x/sz.x), lerp(0,1.5,pos.y/sz.y))
        local p = c
        for i=1,iters do
            p = vec(p.x*p.x - p.y*p.y, 2*p.x*p.y) + c
            if #p > 2 then return i end
        end
        return iters+1
    end)
end

function mb_load() mb = open("mandelbrot.sfi") end
function mb_save() mb:save("mandelbrot.sfi") end

function visualise_simple ()
    (mb/255):save("mandelbrot_simple.png")
end

function visualise_blue1 ()
    mb:map(3, function(m) m=14/m; return vec(1-m,1-m^2,1-m^10) end):save("mandelbrot_blue1.png")
end

function visualise_blue2 ()
    mb:map(3, function(m) m=m/(20+m); return vec(m^10,m^2.5,m) end):save("mandelbrot_blue2.png")
end

print("Run this in the interpreter (luaimg -i -f mandelbrot.lua), then use:")
print("    mb_generate(sz, iters) --params are optional, defaults to 8192,500")
print("    mb_load()")
print("    mb_save()")
print("    visualise_simple()")
print("    visualise_blue1()")
print("    visualise_blue2()")

