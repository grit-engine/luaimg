#!/home/spark/bin/luaimg -F

filenames = { ... }

volume_sz = 256
image_sz = 512

function process_names(list, stats)
    for _,fname in ipairs(list) do
        print("Processing: "..fname)
        open(fname):foreach(function(pix)
            local c = pix * (volume_sz-1) + vector3(0.5, 0.5, 0.5)
            c = vector3(floor(c.x), floor(c.y), floor(c.z))

            local y = c.y + volume_sz * c.z

            local coord = vector2(c.x, y)
            stats:set(coord, stats(coord)+1)
        end)
    end
end

function write_spin (voxel, name, sz)
    local out_images = 72
    for i=0,out_images-1 do
        local filename = string.format("%s-%04d.png",name,i)
        print("Writing "..filename)
        voxel:render(vector2(sz,sz), vector3(15, -(i/out_images * 360)+0.01, 0)):save(filename)
    end
end


stats = make(vector2(volume_sz,volume_sz*volume_sz), 1, 0)
process_names(filenames, stats)
stats = stats / #filenames
stats_tonemapped = stats / (stats + 1)
voxel = make_voxel(stats, volume_sz)
write_spin(voxel,"histo", image_sz)
