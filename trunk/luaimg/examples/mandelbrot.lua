#!../luaimg.linux.x86_64 -F

-- A script for generating mandelbrot images.

mb = nil

function mb_generate (sz, iters, bot_left, top_right)
    sz = sz or vec(8192, 3072)
    iters = iters or 500
    bot_left = bot_left or vec(-2.5, 0)
    top_right = top_right or vec(1.5, 1.5)
    print("Generating mandelbrot set of size: "..sz)
    -- 1 channel texture, no alpha channel
    mb = make(sz, 1, function(pos)
        local c = vec(lerp(bot_left.x, top_right.x, pos.x/sz.x),
                      lerp(bot_left.y, top_right.y, pos.y/sz.y))
        local p = c
        for i=1,iters do
            p = vec(p.x*p.x - p.y*p.y, 2*p.x*p.y) + c
            local len = #p
            if len > 2 then
                return i + 1 - math.log(math.log(len)) / math.log(2)
            end
        end
        return 10000000000000
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
print("    mb_generate(sz, iters) --params are optional, defaults to vec(8192, 3072), 500")
print("    mb_load()")
print("    mb_save()")
print("    visualise_simple()")
print("    visualise_blue1()")
print("    visualise_blue2()")

