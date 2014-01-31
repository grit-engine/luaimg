#!../luaimg.linux.x86_64 -F

if select('#', ...) ~= 1 then
    error("Usage: luaimg -F dds_decompose.lua <ddsfile>")
end
local filename = (...)

function save_mips (filename, mips)
    for _,mip in ipairs(mips) do
        mip:save(("%s.%dx%d.png"):format(filename, mip.width, mip.height))
    end
end

local dds, kind = dds_open(filename)
if kind == "SIMPLE" then
    save_mips(filename, dds)
elseif kind == "CUBE" then
    for face,mips in pairs(dds) do
        save_mips(filename.."."..face, mips)
    end
elseif kind == "VOLUME" then
    for _,mip in ipairs(dds) do
        for z,layer in ipairs(mip) do
            layer:save(("%s.%d.%dx%dx%d.png"):format(filename, z, #mip, layer.width, layer.height))
        end
    end
else
    error("Unrecognised dds kind: "..kind)
end
