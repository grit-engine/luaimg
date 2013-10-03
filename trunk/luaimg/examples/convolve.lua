#!../luaimg -F

lena = open("../test/lena.jpg")

box_blur = make(vec(9,9),1,1):normalise()
lena:convolve(box_blur, true, true):save("box.png")

laplace2 = make(vec(3,3),1,{0,-1,0, -1,4,-1, 0,-1,0}):normalise()
lena:convolve(10*laplace2):save("laplace.png")

horzedge = make(vec(3,1),1,{-1,2,-1}):normalise()
lena:convolve(10*horzedge):save("horzedge.png")

sharpen = laplace2 * 2 + make(vec(3,3),1,function(pos) return pos == vec(1,1) and 1 or 0 end)
lena:convolve(sharpen):save("sharpen.png")


gaussian_blurred = lena:convolveSep(gaussian(7))
gaussian_blurred:save("gaussian.png");
((lena - gaussian_blurred)*10):save("gaussian_diff.png")

unsharp_mask = lena*2 - gaussian_blurred
unsharp_mask:save("unsharpmask.png")
