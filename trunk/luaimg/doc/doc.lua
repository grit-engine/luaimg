include "docgen.lua"

function file_as_string(name)
    local file = io.open(name, "r")
    local str = file:read("*all")
    file:close()
    return str
end

local examples = {

    {
        "Lua programs are also LuaImg programs, so the Lua hello world program stands:",
        false,
[[print "Hello world!"]]
    },

    {
        "Create a 32x32 image with 3 channels containing solid red (vec(1,0,0)).  Save to a file (can also be written on one line):",
        "red.png",
[[my_img = make(vec(32,32), 3, vec(1,0,0))
my_img:save("red.png")]],
    },

    {
        "Initialising a 3 channel image with alpha, initialised with a function (computes an antialiased blue circle with alpha):",
        "circle.png",
[[local sz = vec(64,64)
function init(pos)
    local rad = #(pos - sz/2);
    local alpha = clamp(30-rad, 0, 1)
    return vec(0, 0, 1, alpha)
end
circle = make(sz, 4, true, init)
circle:save("circle.png")]],
    },

    {
        "Blend the circle image onto a solid yellow background:",
        "circle_bg.png",
[[(circle .. vec(1,1,0)):save("circle_bg.png")]],
    },

    {
        "Blend the circle image onto red.png:",
        "circle_bg_red.png",
[[smaller = circle:scale(vec(32,32),"BICUBIC")
blended = smaller .. open("red.png")
blended:save("circle_bg_red.png")]],
    },

    {
        "Extract alpha channel and save it:",
        "circle_a.png",
[[my_mask = circle.w
my_mask:save("circle_a.png")]],
    },

    {
        "Random RGB noise:",
        "random.png",
[[function randvec() return vec(random(), random(), random()) end
my_noise = make(vec(64,64), 3, randvec)
my_noise:save("random.png")]],
    },

    {
        "Random noise with gaussian blur.  The gaussian(n) function returns an nx1 image that represents a separated normalised gaussian blur kernel.  One can also use custom kernels by providing an image instead of using the result of gaussian() -- and these can be provided either in separated form, or as a general rectangular matrix.",
        "perlin.png",
[[my_perlin = my_noise:convolveSep(gaussian(7), true, true)
my_perlin:save("perlin.png")]],
    },

    {
        "Subtract the two to create a high frequency noise texture.  Generally, using arithmetic to combine images is supported.",
        "noise_hifreq.png",
[[((my_noise-my_perlin) + 0.5):save("noise_hifreq.png")]]
    },

}

function generate_imgs()

    for _,v in ipairs(examples) do
        local text, image, code = unpack(v)
        loadstring(code)()
    end

end

function emit_examples(file)
    file:write('    <h2>Script Language Examples</h2>\n')
    file:write('    <div class="prose">\n')

    for _,v in ipairs(examples) do
        local text, image, code = unpack(v)
        file:write('        <p>'..text..'</p>\n')
        if image then
            file:write('        <img src="'..image..'" style="float:right;" alt="LuaImg output" />\n')
        end
        emit_code(file, code)
        file:write('\n')
    end

    file:write("    </div>\n")

end

doc { "function", "make",

[[Create a new image of the specificed size, with the specified number of
channels.  If there is more than 1 channel, the last channel may be an alpha
channel.  Alpha channels behave differently than regular channels.  The init
parameter can be either a single colour (for a solid image), an array of
colours of size W*H, or a function that provides the colour at each pixel.]],

    { "param", "size", "vector2" },
    { "param", "channels", {1,2,3,4} },
    { "param", "alpha", "boolean", optional=true },
    { "param", "init", {"colour", "array[colour]",  "(vector2)->(colour)"} },
    { "return", "Image" },
}

doc { "function", "vec",

[[Convert to a vector value, the number of arguments determine the size of the vector.]],

    { "param", "x", "number" },
    { "param", "y", "number", optional=true },
    { "param", "z", "number", optional=true },
    { "param", "w", "number", optional=true },
    { "return", "vector" },
}

doc { "function", "vec4",

[[Convert to a vector4 value.  The arguments can be either numbers or other
vectors of any size as long as the total number of elements is 2.  There are
similar functions vec2 and vec3 for creating vectors of other sizes.]],

    { "param", "...", {"number","vector","..."} },
    { "return", "vector4" },
}

doc { "function", "open",

[[Load an image file from disk.  The file extension is used to determine
the format.  The extension 'sfi' is a special raw format.  This can be used to
save and restore images in LuaImg's internal representation, but it takes a lot
of space on disk.  All other loading uses libfreeimage.]],

    { "param", "filename", "string" },
    { "return", "Image" },
}

doc { "function", "lerp",

[[Interpolate between two colours / images.  T can be number, vector2/3/4,
or Image.  If lerping images, they must be compatible.]],

    { "param", "v1", "T" },
    { "param", "v2", "T" },
    { "param", "alpha", "number" },
    { "return", "T" },
}

doc { "function", "gaussian",

[[Generate a separated Gaussian convolution kernel.  This is an nx1 image
containing that row of Pascal's triangle, normalised so it all sums to 1.]],

    { "param", "n", "number" },
    { "return", "Image" },
}

doc { "function", "colour",

[[Return a vector value of the given dimensionality, all of whose elements
are the given value.]],

    { "param", "d", "number" },
    { "param", "n", "number" },
    { "return", "vector" },
}

doc { "function", "dot",

[[Compute the dot product of the given vectors.]],

    { "param", "a", "vector" },
    { "param", "b", "vector" },
    { "return", "number" },
}

doc { "function", "cross",

[[Compute the cross product of the given vectors.]],

    { "param", "a", "vector3" },
    { "param", "b", "vector3" },
    { "return", "vector3" },
}

doc { "function", "inv",

[[Invert a quaternion.]],

    { "param", "a", "quat" },
    { "return", "quat" },
}

doc { "function", "slerp",

[[Interpolate between two quaternions.]],

    { "param", "a", "quat" },
    { "param", "b", "quat" },
    { "param", "alpha", "number" },
    { "return", "quat" },
}

doc { "function", "norm",

[[Normalise the vector or quaternion (return a value that has length 1 but is
otherwise equivalent).]],

    { "param", "a", {"vector","quat"} },
    { "return", {"vector","quat"} },
}

doc { "function", "HSLtoRGB",

[[There are 6 functions for converting between RGB, HSV, and HSL.]],

    { "param", "colour", "vector3" },
    { "return", "vector3" },
}

doc {
    "class",
    "Image",

    [[A 2d rectangular grid of pixels.  Pixels are represented in single
precision floating point.  The image can have 1,2,3, or 4 channels.  The last
channel can be an alpha channel (alpha channels have special behaviours when
composing images).  Images can be combined by arithmetic (+,-,*,/,^).  The ..
operator combines images according to alpha blending (i.e. regular blend mode
in Photoshop/Gimp).  Other blend modes are available via the mathematical
operators.  You can therefore mask images using the multiplication operator,
add using the add operator, etc (see examples above).  Individual pixel values
of an image can be accessed using the function call syntax, e.g. img(10,20).
Other functionality is exposed via specific methods on images.  Images can be
swizzled to extract specific channels.  If a swizzle's last character is a
capital letter, this creates an alpha channel.  E.g. img.yX will yield a
greyscale image with alpha channel.  The value channel is the old green channel
and the alpha channel is the old red channel.]],

    { "field", "channels", "number", "The number of channels in the image (including alpha).", },
    { "field", "hasAlpha", "boolean", "Whether or not the last channel is an alpha channel.", },
    { "field", "width", "number", "The number of pixels in a row of the image.", },
    { "field", "height", "number", "The number of pixels in a column of the image.", },
    { "field", "size", "vector2", "The width and height as a single value.", },
    { "field", "numPixels", "vector2", "The width x height.", },
    {
        "method",
        "save",
        "Write the contents of the file to disk, guessing the format from the file extension.",
        { "param", "filename", "string" },
    },
    {
        "method",
        "foreach",
        "Execute the given closure once for each pixel of the image.",
        { "param", "func", "(vector2)->()" },
    },
    {
        "method",
        "map",
        "Create a new image with the given number of channels, the same size as this image, initialised by executing the function provided to map each pixel from this image into the new image.  The function is called with both the colour from the existing image, and the coordinate being set (like make).",
        { "param", "channels", "number" },
        { "param", "alpha", "boolean", optional=true },
        { "param", "func", "(colour, vector2)->(colour)" },
        { "return", "Image" },
    },
    {
        "method",
        "reduce",
        "Compute a single value from this image in a generic fashion.  The computed value has the same number of channels as the image.  The given function is called for each pixel.  It is provided with the old running total, the current pixel value, and the current pixel position.  It is expected to return the new running total.",
        { "param", "zero", "colour" },
        { "param", "func", "(colour,colour,vector2)->(colour)" },
        { "return", "colour" },
    },
    {
        "method",
        "crop",
        "Create a new image of the given size that is initialised to a copied version of this image, or the background colour if the pixel is not within the bounds of this image.  The background colour defaults to black.",
        { "param", "bottom_left", "vector2" },
        { "param", "size", "vector2" },
        { "param", "background", "colour", optional=true },
        { "return", "Image" },
    },
    {
        "method",
        "scale",
        "Create a new image the same as this one but a different size.  The available filter methods are BOX, BILINEAR, BSPLINE, BICUBIC, CATMULLROM, and LANCZOS3.",
        { "param", "size", "vector2" },
        { "param", "filter", "string" },
        { "return", "Image" },
    },
    {
        "method",
        "rotate",
        "Create a new image the same as this one but rotated by the given angle (degrees).  The resulting image will have larger area if the angle is not a multiple of 90.",
        { "param", "angle", "number" },
        { "return", "Image" },
    },
    {
        "method",
        "clone",
        "Create a new image identical to this one.  This is useful if you then modify it with set, drawImage, etc.",
        { "return", "Image" },
    },
    {
        "method",
        "mirror",
        "Create a new image identical to this one but inverted on the X axis.",
        { "return", "Image" },
    },
    {
        "method",
        "flip",
        "Create a new image identical to this one but inverted on the Y axis.",
        { "return", "Image" },
    },
    {
        "method",
        "meanDiff",
        "The average difference between pixels in two compatible images.",
        { "param", "other", "Image" },
        { "return", "colour" },
    },
    {
        "method",
        "rmsDiff",
        "Subtract one image from the other.  Square every pixel channel value, average them all, and square root the result.  This is a common method for objectively measuring the difference between two compatible images.  The resulting value is a colour, but you can use the # operator to reduce this to a single value, e.g. #img:rms(other).",
        { "param", "other", "Image" },
        { "return", "colour" },
    },
    {
        "method",
        "set",
        "Assign a given colour to a single pixel coordinate.",
        { "param", "pos", "vector2" },
        { "param", "colour", "colour" },
    },
    {
        "method",
        "max",
        "The returned image is the max of the two given images (they must be compatible).  The max of two pixels is the max of each of their channels.  This is sometimes called 'lighten only'.  It is also allowed to give a single colour value in place of the other image.",
        { "param", "other", "Image/colour" },
        { "return", "Image" },
    },
    {
        "method",
        "min",
        "The returned image is the min of the two given images (they must be compatible).  The min of two pixels is the min of each of their channels.  This is sometimes called 'darken only'.  It is also allowed to give a single colour value in place of the other image.",
        { "param", "other", "Image/colour" },
        { "return", "Image" },
    },
    {
        "method",
        "abs",
        "The returned image is the absolute value of this image, i.e. negative pixel channel values are made positive.",
        { "return", "Image" },
    },
    {
        "method",
        "convolve",
        "Perform a convolution operation on this image, using the given kernel, to yield a new image.  The kernel must have a single channel and have an odd width and height.  The wrapx and wrapy control the behaviour at the edge of the image and default to false.  When not wrapping, the effect is to 'clamp' the lookups at the pixel border.",
        { "param", "kernel", "Image" },
        { "param", "wrapx", "boolean", optional=true },
        { "param", "wrapy", "boolean", optional=true },
        { "return", "Image" },
    },
    {
        "method",
        "convolveSep",
        "Perform a convolution operation on this image, using the given separable kernel, to yield a new image.  The kernel must have a single channel, an odd width, and a height of 1.  The kernel is used to first convolve horizontally, and then vertically.  This is often more efficient than creating a square kernel and doing a single convolution operation.  The wrapx and wrapy control the behaviour at the edge of the image and default to false.  When not wrapping, the effect is to 'clamp' the lookups at the pixel border.",
        { "param", "kernel", "Image" },
        { "param", "wrapx", "boolean", optional=true },
        { "param", "wrapy", "boolean", optional=true },
        { "return", "Image" },
    },
    {
        "method",
        "normalise",
        "Return a new image where the sum of all the positive pixel channels is 1, and the sum of all the negative pixel channels is -1.  This is useful for normalising kernels for convolutions, so that the overall brightness of the image is unchanged during the convolution.",
        { "return", "Image" },
    },
    {
        "method",
        "drawImage",
        "Draw another image on top of this image, in the position given.  Both images must have the same number of non-alpha channels.  The image being drawn on top must have an alpha channel but this image need not.  The two boolean parameters cause the other image to wrap around this image if it is drawn at the edges.  This is useful when creating wrappable textures.  The wrap parameters both default to false.",
        { "param", "other", "Image" },
        { "param", "bottom_left", "vector2" },
        { "param", "wrap_x", "boolean", optional=true },
        { "param", "wrap_y", "boolean", optional=true },
    },
    {
        "method",
        "drawImageAt",
        "The same as drawImage but draws the image centered at the given location instead of with its bottom left corner at that location.  ",
        { "param", "other", "Image" },
        { "param", "pos", "vector2" },
        { "param", "wrap_x", "boolean", optional=true },
        { "param", "wrap_y", "boolean", optional=true },
    },
}


function emit_title(file, title)
    file:write("    <div class='titleblock'>")
    file:write("        <img class='logo' src='logo.png' alt='logo' />")
    file:write("        <h1>"..title.."</h1>")
    file:write("        <div class='toplinks'>")
    file:write("            <a class='toplink' href='index.html'>Overview</a>")
    file:write("            <a class='toplink' href='examples.html'>Examples</a>")
    file:write("            <a class='toplink' href='download.html'>Download</a>")
    file:write("            <a class='toplink' href='usage.html'>Usage</a>")
    file:write("            <a class='toplink' href='api.html'>API</a>")
    file:write("        </div>")
    file:write("    </div>")
end
    
file = io.open("index.html","w")
file:write(file_as_string("header.html"))
emit_title(file, "LuaImg")
file:write(file_as_string("index_content.html"))
file:write(file_as_string("footer.html"))
file:close()

file = io.open("examples.html","w")
file:write(file_as_string("header.html"))
emit_title(file, "LuaImg")
emit_examples(file)
file:write(file_as_string("footer.html"))
file:close()

file = io.open("download.html","w")
file:write(file_as_string("header.html"))
emit_title(file, "LuaImg")
file:write(file_as_string("download_content.html"))
file:write(file_as_string("footer.html"))
file:close()

file = io.open("usage.html","w")
file:write(file_as_string("header.html"))
emit_title(file, "LuaImg")
file:write(file_as_string("usage_content.html"))
file:write(file_as_string("footer.html"))
file:close()

file = io.open("api.html","w")
file:write(file_as_string("header.html"))
emit_title(file, "LuaImg")
file:write(file_as_string("api_content.html"))
emit_api(file)
file:write(file_as_string("footer.html"))
file:close()

generate_imgs()

include "../examples/logo.lua"

-- Actually prepare zips
-- gc problem
-- drawImage that wraps
-- draw line
