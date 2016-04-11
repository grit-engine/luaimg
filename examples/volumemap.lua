#!../luaimg.linux.x86_64 -F 

sz = vec(32, 32, 32)

function xyz_to_colour(pos)
        pos = norm(pos)
        local tpos = quat(37, norm(vec(0.15, 0.45, 0.38))) * pos
        local az = deg(atan2(tpos.x, tpos.y))
        local el = deg(acos(tpos.z))
        local closest_az = floor(az/18+0.5)*18
        local col = vec(0,0,0)
        if abs(az - closest_az) < 1 then
                col = col + (1-min(1,abs(az - closest_az) / 1)) * vec(1,0,0)
        end
        local closest_el = floor(el/18+0.5)*18
        if abs(el - closest_el) < 0.5 then
                col = col + (1-min(1,abs(el - closest_el) / 0.5)) * vec(0,1,0)
        end
        if dot(pos, vec(1,0,0)) > 0.9 then col = col + vec(1,0,0) end
        if dot(pos, vec(0,1,0)) > 0.9 then col = col + vec(0,1,0) end
        if dot(pos, vec(0,0,1)) > 0.9 then col = col + vec(0,0,1) end
        if dot(pos, vec(-1,0,0)) > 0.9 then col = col + vec(0,1,1) end
        if dot(pos, vec(0,-1,0)) > 0.9 then col = col + vec(1,0,1) end
        if dot(pos, vec(0,0,-1)) > 0.9 then col = col + vec(1,1,0) end
        return vec4(col,1)
end

print('Creating a procedural texture of size ' .. sz)

volume = {}
for i=1,sz.z do
        volume[i] = make(sz.xy, 3, true, function(pos)
                local uvw = vec3(pos, i)
                return xyz_to_colour((uvw-(sz-1)/2)/(sz/2))
        end)
        io.stdout:write('.')
        io.stdout:flush()
end

io.stdout:write('\n')

print 'Generating mipmaps...' 

vmipmaps = volume_mipmaps(volume)

print 'Saving to disk...' 

dds_save_volume("volume.dds", "BC1", vmipmaps)
