#!../luaimg -F

with_names = {

    "nl/with/1200.jpg", "nl/with/1201.jpg", "nl/with/1202.jpg", "nl/with/1203.jpg",
    "nl/with/1204.jpg", "nl/with/1205.jpg", "nl/with/1206.jpg", "nl/with/1207.jpg",
    "nl/with/1208.jpg", "nl/with/1209.jpg", "nl/with/1210.jpg", "nl/with/1216.jpg",
    "nl/with/1223.jpg", "nl/with/1224.jpg", "nl/with/1225.jpg", "nl/with/1226.jpg",
    "nl/with/1227.jpg", "nl/with/1228.jpg", "nl/with/1231.jpg", "nl/with/1232.jpg",
    "nl/with/1235.jpg", "nl/with/1238.jpg", "nl/with/1240.jpg", "nl/with/1241.jpg",
    "nl/with/1242.jpg", "nl/with/1243.jpg", "nl/with/1244.jpg", "nl/with/1245.jpg",
    "nl/with/1246.jpg", "nl/with/1251.jpg", "nl/with/1252.jpg", "nl/with/1217.jpg",
    "nl/with/1222.jpg", "nl/with/1229.jpg", "nl/with/1220.jpg", "nl/with/1221.jpg", 
    "nl/with/1230.jpg", "nl/with/1253.jpg", 

}

without_names = {

    "nl/without/1211.jpg", "nl/without/1212.jpg", "nl/without/1213.jpg",
    "nl/without/1214.jpg", "nl/without/1215.jpg", "nl/without/1218.jpg",
    "nl/without/1219.jpg","nl/without/1233.jpg", "nl/without/1234.jpg",
    "nl/without/1236.jpg", "nl/without/1237.jpg", "nl/without/1239.jpg",
    "nl/without/1247.jpg", "nl/without/no_1200.jpg", "nl/without/no_1201.jpg",
    "nl/without/no_1202.jpg", "nl/without/no_1203.jpg", "nl/without/no_1204.jpg",
    "nl/without/no_1205.jpg", "nl/without/no_1206.jpg", "nl/without/no_1207.jpg",
    "nl/without/no_1208.jpg", "nl/without/no_1209.jpg", "nl/without/no_1210.jpg",
    "nl/without/no_1211.jpg", "nl/without/no_1212.jpg", "nl/without/no_1213.jpg",
    "nl/without/no_1214.jpg", "nl/without/no_1215.jpg", "nl/without/no_1216.jpg",
    "nl/without/no_1217.jpg", "nl/without/no_1218.jpg", "nl/without/no_1219.jpg",
    "nl/without/no_1220.jpg", "nl/without/no_1221.jpg", "nl/without/no_1222.jpg",
    "nl/without/no_1223.jpg", "nl/without/no_1224.jpg", "nl/without/no_1225.jpg",
    "nl/without/no_1226.jpg", "nl/without/no_1227.jpg", "nl/without/no_1228.jpg",
    "nl/without/no_1229.jpg", "nl/without/no_1230.jpg", "nl/without/no_1231.jpg",
    "nl/without/no_1232.jpg", "nl/without/no_1233.jpg", "nl/without/no_1235.jpg",
    "nl/without/no_1236.jpg", "nl/without/no_1237.jpg", "nl/without/no_1238.jpg",
    "nl/without/no_1239.jpg", "nl/without/no_1240.jpg", "nl/without/no_1241.jpg",
    "nl/without/no_1242.jpg", "nl/without/no_1243.jpg", "nl/without/no_1244.jpg",
    "nl/without/no_1245.jpg", "nl/without/no_1246.jpg", "nl/without/no_1247.jpg",
    "nl/without/no_1248.jpg", "nl/without/no_1249.jpg", "nl/without/no_1250.jpg",
    "nl/without/no_1251.jpg", "nl/without/no_1253.jpg", "nl/without/no_1254.jpg",
    "nl/without/no_1255.jpg", "nl/without/no_1256.jpg", "nl/without/no_1257.jpg",
    "nl/without/no_1258.jpg", "nl/without/no_1259.jpg", "nl/without/no_1260.jpg",
    "nl/without/no_1261.jpg", "nl/without/no_1262.jpg", "nl/without/no_1263.jpg",

}

function process_names(list, stats)
    for _,fname in ipairs(list) do
        local counter = 0
        open(fname):foreach(function(pix)
            local r, g, b = pix.x, pix.y, pix.z;
            local qty1 = g - (0.95*b + 0.1)
            local qty2 = g - (0.95*r + 0.1)
            local qty = min(qty1, qty2)
            if qty > 0 then
                counter = counter + qty
            end
        end)
        print(fname..": "..floor(counter))
    end
end

function map_names(list, stats)
    for _,fname in ipairs(list) do
        local counter = 0
        open(fname):map(3,function(pix)
            local r, g, b = pix.x, pix.y, pix.z;
            local qty1 = g - (0.95*b + 0.06)
            local qty2 = g - (0.95*r + 0.06)
            local qty = min(qty1, qty2)
            qty = pow(qty,0.2)
            if qty > 0 then
                counter = counter + qty
                return vector3(1,qty,qty)
            else
                return pix
            end
        end):save(fname..".processed.jpg")
        collectgarbage("collect")
        print(fname..": "..floor(counter).." "..(counter > 25000 and "YES" or "NO"))
    end
end

--process_names(with_names, with_stats)
--process_names(without_names, without_stats)
map_names(with_names, with_stats)
map_names(without_names, without_stats)
