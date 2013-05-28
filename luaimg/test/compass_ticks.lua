#!../luaimg -F 

-- drawn at this size
local sz = vector2(2048, 2048)

-- scaled down to this size to avoid aliasing
local out_sz = vector2(128, 128)

make(sz, 3, function(pos)

    -- convert pixel position from pixel coords to [-1, 1] range
    pos = pos / (sz - vector2(1,1)) * 2 - vector2(1, 1)

    -- calculate angle from top, [0, 360) range
    local angle = (deg(atan2(pos.x, pos.y)) + 360) % 360

    -- one tick every 10 degrees, this gives the index of the nearest tick
    local tick = floor(angle / 10 + 0.5)

    -- what actual angle the nearest tick is
    local tick_angle = tick*10

    if abs(angle - tick_angle) < 1 then

        -- angle is within 1 degree of the tick angle

        -- default radius boundaries
        local inside, outside = 0.85, 0.95

        if mod(tick,3) == 0 then
            -- tick index is a multiple of 3, make it longer
            inside = 0.75; outside = 1
        end

        -- calculate radius as length of pos
        local rad = #pos

        if rad>inside and rad<outside then
            -- we're a tick pixel
            return vector3(1,1,1)
        end
    end

    -- we're not a tick pixel
    return vector3(0,0,0)

end):scale(out_sz, "LANCZOS3"):save("compass_ticks.png")
