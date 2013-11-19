#!../luaimg.linux.x86_64 -F

-- Create a popular meme (image with white text and black shadow at the bottom)

img = open("lena.jpg")

-- black/white image containing text (font location is hard-coded)
txt = text("/usr/share/fonts/truetype/msttcorefonts/Impact.ttf", vec(40,40), "Classic Lena image ╬ψφ£")

--form shadow by blurring (and intensifying) the text (note: using crop to enlarge it to allow space for the blur)
shadow = (5*txt:crop(vec(-8,-8), txt.size+vec(16,16),0)):convolveSep(gaussian(15))

-- centre of text
pos = vec(img.width/2,60)

-- draw shadow first (convert to black image with blurred channel as alpha channel)
img:drawImageAt(shadow.eeeX, pos)
-- then draw the text itself (convert to white image with original text channel as alpha channel)
img:drawImageAt(txt.fffX, pos)

img:save("meme.png")
