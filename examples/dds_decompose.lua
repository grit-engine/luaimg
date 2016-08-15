#!../luaimg.linux.x86_64 -F

if select('#', ...) ~= 1 then
    error("Usage: luaimg -F dds_decompose.lua <ddsfile>")
end
local filename = (...)

function format_from_max(n)
    assert(n > 0)
    return "%0" .. (floor(log(n) / log(10)) + 1) .. "d"
end

function save_mips (filename, max_sz, mips)
    sz_fmt = format_from_max(max_sz)
    mip_fmt = format_from_max(#mips)
    for mip_i, mip in ipairs(mips) do
        mip:save(("%s.m"..mip_fmt.."."..sz_fmt.."x"..sz_fmt..".png"):format(filename, mip_i - 1, mip.width, mip.height))
    end
end

local dds, kind = dds_open(filename)
if kind == "SIMPLE" then
    max_sz = max(dds[1].width, dds[1].height)
    save_mips(filename, max_sz, dds)
elseif kind == "CUBE" then
    for face, mips in pairs(dds) do
        max_sz = max(mips[1].width, mips[1].height)
        save_mips(filename.."."..face, max_sz, mips)
    end
elseif kind == "VOLUME" then
    mip_fmt = format_from_max(#dds)
    max_sz = max(dds[1][1].width, dds[1][1].height, #dds[1])
    sz_fmt = format_from_max(max_sz)
    for mip_i, mip in ipairs(dds) do
        for z, layer in ipairs(mip) do
            layer:save(("%s.m"..mip_fmt.."."..sz_fmt.."."..sz_fmt.."x"..sz_fmt.."x"..sz_fmt..".png"):format(filename, mip_i - 1, z, #mip, layer.width, layer.height))
        end
    end
else
    error("Unrecognised dds kind: "..kind)
end
