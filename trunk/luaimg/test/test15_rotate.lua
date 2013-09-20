#!../luaimg -F

a = open("lena.jpg")
a:rotate(0):save("output_test15_rotate0.png")
a:rotate(360):save("output_test15_rotate360.png")
a:rotate(450):save("output_test15_rotate450.png")
a:rotate(90):save("output_test15_rotate90.png")
a:rotate(180):save("output_test15_rotate180.png")
a:rotate(270):save("output_test15_rotate270.png")
a:rotate(45):save("output_test15_rotate45.png")
a:rotate(-45):save("output_test15_rotate-45.png")
a:rotate(1):save("output_test15_rotate-45.png")
