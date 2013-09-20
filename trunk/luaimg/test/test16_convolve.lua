#!../luaimg -F

lena = open("lena.jpg")

box_blur = make(vector2(9,9),1,1):normalise()
lena:convolve(box_blur, true, true):save("test16_box1.png")
lena:convolve(box_blur):save("output_test16_box2.png")

laplace2 = make(vector2(3,3),1,{0,-1,0, -1,4,-1, 0,-1,0}):normalise()
lena:convolve(10*laplace2):save("output_test16_laplace.png")

horzedge = make(vector2(3,1),1,{-1,2,-1}):normalise()
lena:convolve(10*horzedge):save("output_test16_horzedge.png")

sharpen = laplace2 * 2 + make(vector2(3,3),1,function(pos) return pos == vector2(1,1) and 1 or 0 end)
lena:convolve(sharpen):save("output_test16_sharpen.png")


gaussian = make(vector2(7,7),1, {
    0.00000067, 0.00002292, 0.00019117, 0.00038771, 0.00019117, 0.00002292, 0.00000067,
    0.00002292, 0.00078634, 0.00655965, 0.01330373, 0.00655965, 0.00078633, 0.00002292,
    0.00019117, 0.00655965, 0.05472157, 0.11098164, 0.05472157, 0.00655965, 0.00019117,
    0.00038771, 0.01330373, 0.11098164, 0.22508352, 0.11098164, 0.01330373, 0.00038771,
    0.00019117, 0.00655965, 0.05472157, 0.11098164, 0.05472157, 0.00655965, 0.00019117,
    0.00002292, 0.00078633, 0.00655965, 0.01330373, 0.00655965, 0.00078633, 0.00002292,
    0.00000067, 0.00002292, 0.00019117, 0.00038771, 0.00019117, 0.00002292, 0.00000067,
})
gaussian_blurred = lena:convolve(gaussian)
gaussian_blurred:save("output_test16_gaussian.png");
((lena - gaussian_blurred)*vector3(10,10,10)):save("output_test16_gaussian_diff.png")

unsharp_mask = make(gaussian.size,1,function(pos) return pos == vector2(3,3) and 2 or 0 end) - gaussian ;
lena:convolve(unsharp_mask):save("output_test16_unsharpmask.png")

sep_gaussian_x = make(vector2(7,1),1,{ 1, 6, 15, 20, 15, 6, 1 }):normalise()
sep_gaussian_y = sep_gaussian_x:rotate(90)
lena:convolve(sep_gaussian_x):convolve(sep_gaussian_y):save("output_test16_separated_gaussian.png")
