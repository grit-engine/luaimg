#!../luaimg -F

lena = open("lena.jpg")
box_blur = make(vector2(9,9),1,1):normalise()
lena:convolve(box_blur, true, true):save("test16_box1.png")
lena:convolve(box_blur, false, false):save("test16_box2.png")

laplace2 = make(vector2(3,3),1,0)
laplace2:set(vector2(1,1),  4)
laplace2:set(vector2(0,1), -1)
laplace2:set(vector2(1,0), -1)
laplace2:set(vector2(2,1), -1)
laplace2:set(vector2(1,2), -1)
laplace2 = laplace2:normalise()
lena:convolve(10*laplace2, false, false):save("test16_laplace.png")

horzedge = make(vector2(3,1),1,0)
horzedge:set(vector2(0,0), -1)
horzedge:set(vector2(1,0),  2)
horzedge:set(vector2(2,0), -1)
horzedge = horzedge:normalise()
lena:convolve(10*horzedge, false, false):save("test16_horzedge.png")

sharpen = laplace2 * 2 + make(vector2(3,3),1,function(pos) return pos == vector2(1,1) and 1 or 0 end)
lena:convolve(sharpen, false, false):save("test16_sharpen.png")


gaussian_numbers = {
    {0.00000067, 0.00002292, 0.00019117, 0.00038771, 0.00019117, 0.00002292, 0.00000067},
    {0.00002292, 0.00078634, 0.00655965, 0.01330373, 0.00655965, 0.00078633, 0.00002292},
    {0.00019117, 0.00655965, 0.05472157, 0.11098164, 0.05472157, 0.00655965, 0.00019117},
    {0.00038771, 0.01330373, 0.11098164, 0.22508352, 0.11098164, 0.01330373, 0.00038771},
    {0.00019117, 0.00655965, 0.05472157, 0.11098164, 0.05472157, 0.00655965, 0.00019117},
    {0.00002292, 0.00078633, 0.00655965, 0.01330373, 0.00655965, 0.00078633, 0.00002292},
    {0.00000067, 0.00002292, 0.00019117, 0.00038771, 0.00019117, 0.00002292, 0.00000067},
}
gaussian = make(vector2(7,7),1,function(pos)return gaussian_numbers[pos.y+1][pos.x+1] end)
gaussian_blurred = lena:convolve(gaussian, false, false)
gaussian_blurred:save("test16_gaussian.png");
((lena - gaussian_blurred)*vector3(10,10,10)):save("test16_gaussian_diff.png")

unsharp_mask = make(vector2(7,7),1,function(pos) return pos == vector2(3,3) and 2 or 0 end) - gaussian ;
lena:convolve(unsharp_mask, false, false):save("test16_unsharpmask.png")

sep_gaussian_numbers = { 1, 6, 15, 20, 15, 6, 1 }
sep_gaussian_x = make(vector2(7,1),1,function(pos)return sep_gaussian_numbers[pos.x+1] end):normalise()
sep_gaussian_y = sep_gaussian_x:rotate(90)
lena:convolve(sep_gaussian_x, false, false):convolve(sep_gaussian_y, false, false):save("test16_separated_gaussian.png")
