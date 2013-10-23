#!../luaimg -F

-- Creates the familiar pattern of artifacts due to the Bresenham line drawing algorithm.

local company_colour = vec(48,80,112)/255
local line_colour = vec(1,1,1,1)

step = 4
img = make(vec(257,257),3,company_colour)
c = img.size/2

for i=0,img.height-1,step do
    img:drawLine(vec(0,i), c, 1, line_colour)
    img:drawLine(vec(img.width-1,i), c, 1, line_colour)
end
for i=0,img.width-1,step do
    img:drawLine(vec(i,0), c, 1, line_colour)
    img:drawLine(vec(i,img.height-1), c, 1, line_colour)
end

img:save("bresenham_pattern.png")
