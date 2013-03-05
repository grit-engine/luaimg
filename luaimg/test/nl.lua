#!../luaimg -F

northern_lights_threshold = 25000

function is_northern_lights(pix)
    local r, g, b = pix.x, pix.y, pix.z;
    local qty1 = g - (0.95*b + 0.06)
    local qty2 = g - (0.95*r + 0.06)
    local qty = min(qty1, qty2)
    qty = pow(qty,0.2)
    return qty
end

function process_names(list)
    for _,fname in ipairs(list) do
        local counter = 0
        open(fname):foreach(function(pix)
            local qty = is_northern_lights(pix)
            if qty > 0 then
                counter = counter + qty
            end
        end)
        print(fname..": "..floor(counter).." "..(counter > northern_lights_threshold and "YES" or "NO"))
    end
end

function map_names(list)
    for _,fname in ipairs(list) do
        local counter = 0
        open(fname):map(3,function(pix)
            local qty = is_northern_lights(pix)
            if qty > 0 then
                counter = counter + qty
                return vector3(1,qty,qty)
            else
                return pix
            end
        end):save(fname..".processed.jpg")
        collectgarbage("collect")
        print(fname..": "..floor(counter).." "..(counter > northern_lights_threshold and "YES" or "NO"))
    end
end

process_names({...})
--map_names(with_names)
--map_names(without_names)
