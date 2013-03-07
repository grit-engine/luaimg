#!../luaimg -F

make(vector2(256,256*256),3,function(pos) local y,z = pos.y%256,floor(pos.y/256); return vector3(pos.x,y,z)/256 end):save("output_test8.png")

