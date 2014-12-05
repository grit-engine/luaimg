#!../luaimg.linux.x86_64 -F

include "docgen.lua"

if select("#",...) ~= 3 then
    print "Usage: ./doc.lua <validator> <analytics> <images>"
    os.exit(1)
end
should_emit_validator = "true" == (select(1,...))
should_emit_analytics = "true" == (select(2,...))
should_emit_images = "true" == (select(3,...))

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
        "Draw a black border around that image and a few white pixels in it.",
        "redb.png",
[[my_img:drawLine(vec(0,0),vec(0,31),1,vec(0,0,0))
my_img:drawLine(vec(0,0),vec(31,0),1,vec(0,0,0))
my_img:drawLine(vec(31,31),vec(0,31),1,vec(0,0,0))
my_img:drawLine(vec(31,31),vec(31,0),1,vec(0,0,0))
my_img:draw(vec(10,15),vec(1,1,1))
my_img:draw(vec(15,15),vec(1,1,1))
my_img:draw(vec(15,10),vec(1,1,1))
my_img:save("redb.png")]],
    },

    {
        "Initialising a 3 channel + alpha image using a function (computes an antialiased blue circle with alpha):",
        "circle.png",
[[local sz = vec(64,64)
function init(pos)
    local rad = #(pos - sz/2);
    local alpha = clamp(30-rad, 0, 1)
    return vec(0, 0, 1, alpha)
end
circle = make(sz, 3, true, init)
circle:save("circle.png")]],
    },

    {
        "From the circle image, rotate the colour channels (preserving the alpha), and blend onto a solid yellow background, then enlarge it to get a black border:",
        "circle_bg.png",
[[img = (circle.zxyW .. vec(1,1,0)):crop(vec(-1,-1), vec(66,66), 0)
img:save("circle_bg.png")]],
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


-- {{{ General Utilities

doc { "function", "seconds", module="General Utilities",

[[Return the number of seconds since reboot.  Useful for benchmarking.]],

    { "return", "number" },
}

doc { "function", "vec", module="General Utilities",

[[Convert to a vector value, the number of arguments determines the number of
dimensions of the vector.]],

    { "param", "x", "number" },
    { "param", "y", "number", optional=true },
    { "param", "z", "number", optional=true },
    { "param", "w", "number", optional=true },
    { "return", "vector" },
}

doc { "function", "vec4", module="General Utilities",

[[Convert to a vector4 value.  The arguments can be either numbers or other
vectors of any size as long as the total number of elements is 2.  There are
similar functions vec2 and vec3 for creating vectors of other sizes.]],

    { "param", "...", {"number","vector","..."} },
    { "return", "vector4" },
}

doc { "function", "colour", module="General Utilities",

[[Return a vector value of the given dimensionality, all of whose elements
are the given value.]],

    { "param", "d", "number" },
    { "param", "n", "number" },
    { "return", "vector" },
}

doc { "function", "dot", module="General Utilities",

[[Compute the dot product of the given vectors.]],

    { "param", "a", "vector" },
    { "param", "b", "vector" },
    { "return", "number" },
}

doc { "function", "cross", module="General Utilities",

[[Compute the cross product of the given vectors.]],

    { "param", "a", "vector3" },
    { "param", "b", "vector3" },
    { "return", "vector3" },
}

doc { "function", "inv", module="General Utilities",

[[Invert a quaternion.]],

    { "param", "a", "quat" },
    { "return", "quat" },
}

doc { "function", "slerp", module="General Utilities",

[[Interpolate between two quaternions.]],

    { "param", "a", "quat" },
    { "param", "b", "quat" },
    { "param", "alpha", "number" },
    { "return", "quat" },
}

doc { "function", "norm", module="General Utilities",

[[Normalise the vector or quaternion (return a value that has length 1 but is
otherwise equivalent).]],

    { "param", "a", {"vector","quat"} },
    { "return", {"vector","quat"} },
}

doc { "function", "HSLtoRGB", module="General Utilities",

[[There are 6 functions for converting between the various combinations of RGB,
HSV, and HSL.  HSV is distinguished from HSL because in HSL, 100% brightness is
white (i.e. it desaturates), whereas in HSV it retains saturation.]],

    { "param", "colour", "vector3" },
    { "return", "vector3" },
}

-- }}}

-- {{{ Disk I/O

doc { "function", "open", module="Disk I/O",

[[Load an image file from disk.  The file extension is used to determine the
format.  The extension 'sfi' is a special raw format.  This can be used to save
and restore images in LuaImg's internal representation, which is 4 bytes per
pixel per channel.  All other formats are loaded with libfreeimage.]],

    { "param", "filename", "string" },
    { "return", "Image" },
}

doc { "function", "dds_open", module="Disk I/O",

[[Load a dds (Direct Draw Surface) file from disk.  This format is different
from the other supported formats because it is a package of image files with
associated metadata.  The first return value is the content of the dds file,
which can consist of more than one image.  The second is a string indicating
the kind of file that was opened, of which there are 3 possibilities.</p><p>If
the kind is SIMPLE, then the dds file was a simple 2D texture so the content is
a table containing the mipmaps (in descending order of size).  If the kind is
CUBE, then the table contains 6 sides of this cube, e.g. in the field px is the
positive x side, nx is the negative x side, and similarly for the y and z axes.
Such textures are typically used for 360 degree panoramas.  Finally, if the
kind is VOLUME, then this is a 3D texture.  The content is a table of volume
mipmaps, each of which is a table containing all the single images that
comprise the volume, one slice at a time.</p><p>BC1-5 are supported, as
well as the basic raw formats.]],

    { "param", "filename", "string" },
    { "return", "table" },
    { "return", { "\"SIMPLE\"", "\"CUBE\"", "\"VOLUME\"" } },
}

doc { "function", "dds_save_simple", module="Disk I/O",

[[Save a simple dds (Direct Draw Surface) file to disk.  This will save a 2D
texture, there are 2 other functions for saving cube maps and volume maps.  The
available formats are R5G6B5, R8G8B8, A8R8G8B8, A2R10G10B10, A1R5G5B5, R8, R16,
G16R16, A8R8, A4R4, A16R16, R3G3B2, A4R4G4B4, and BC1-5.  You must give an
array of mipmaps to this function.  If you only want to save the top mipmap
then use a single element array.  You can also use the mipmaps() function to
generate mipmaps for you.</p><p>Note that the supplied images must have the right
number of channels/alpha for the chosen format.  Don't forget that BC1 has an
alpha channel.  To add a 100% alpha channel to an RGB image, use the img.xyzF
swizzle.]],

    { "param", "filename", "string" },
    { "param", "format", "string" },
    { "param", "mipmaps", "array of images" },
}

doc { "function", "dds_save_cube", module="Disk I/O",

[[Save a cubemap dds (Direct Draw Surface) file to disk.  This behaves much like dds_save_simple() except all sides of the cube must be supplied.]],

    { "param", "filename", "string" },
    { "param", "format", "string" },
    { "param", "pos_x", "array of images" },
    { "param", "neg_x", "array of images" },
    { "param", "pos_y", "array of images" },
    { "param", "neg_y", "array of images" },
    { "param", "pos_z", "array of images" },
    { "param", "neg_z", "array of images" },
}

doc { "function", "dds_save_volume", module="Disk I/O",

[[Save a volume dds (Direct Draw Surface) file to disk.  This behaves much like
dds_save_simple() except each mipmap is a table of slices (images), thus the
input is an array of arrays of images.  If you do not want images, give a
singleton array containing the array of the slices in your volume.  You can
also use volume_mipmaps() to generate the array of arrays from an array of
images.]],

    { "param", "filename", "string" },
    { "param", "format", "string" },
    { "param", "mipmaps", "array of arrays of images" },
}

doc { "function", "gif_open", module="Disk I/O",

[[Open a gif file.  While the regular open() call supports gifs, this function additionally supports
reading the various frames from animated gifs.  Returned are 3 values, the first is the number of
loops (0 to 56636 inclusive, 0 meaning infinite looping).  The second is an array of images, one for
each frame, and the final value is a table of delays (in seconds).]],

    { "param", "filename", "string" },
}

doc { "function", "gif_save", module="Disk I/O",

[[Save a gif file.  While gifs can be saved with the save() method of an image, this call allows
saving many frames to create an animated gif.  Specifying 0 loops means infinite looping.  The
images must all be the same size and have 3 channels (with optional alpha).  Giving a single delay
means the same delay is used for all frames, or an array can be used to give a different delay for
each frame. In either case, the delays are in seconds, must be given in multiples of 0.01, and
values less than 0.02 are not well-supported by browsers..]],

    { "param", "filename", "string" },
    { "param", "loops", "number" },
    { "param", "frames", "array of images" },
    { "param", "delays", {"number", "array of numbers" } },
}

-- }}}

-- {{{ Image Globals

doc { "function", "make", module="Image Globals",

[[Create a new image of the specificed size, with the specified number of
colour channels.  If channels&lt;4, one can also add an alpha channel.  Alpha
channels behave differently than regular channels.  The init parameter can be
either a single colour (for a solid image), an array of colours of size W*H, or
a function that provides the colour at each pixel.]],

    { "param", "size", "vector2" },
    { "param", "channels", {1,2,3,4} },
    { "param", "alpha", "boolean", optional=true },
    { "param", "init", {"colour", "array[colour]",  "(vector2)->(colour)"} },
    { "return", "Image" },
}

doc { "function", "lerp", module="Image Globals",

[[Interpolate between two colours / images.  T can be number, vector2/3/4, or
Image.  If lerping images, they must be compatible.]],

    { "param", "v1", "T" },
    { "param", "v2", "T" },
    { "param", "alpha", "number" },
    { "return", "T" },
}

doc { "function", "gaussian", module="Image Globals",

[[Generate a separated Gaussian convolution kernel.  This is an nx1 image
containing that row of Pascal's triangle, normalised so it all sums to 1.]],

    { "param", "n", "number" },
    { "return", "Image" },
}

doc { "function", "mipmaps", module="Image Globals",

[[Takes an image, and creates an array of scaled versions of the image, where
each successive image is half the size of the previous one in both dimensions.
This is mainly useful for the dds_save family of functions.  The available
filters are the same as for the image:scale() method.]],

    { "param", "img", "Image" },
    { "param", "filter", "string" },
    { "return", "array of Images" },
}

doc { "function", "volume_mipmaps", module="Image Globals",

[[Takes a volume (an array of images), and creates an array of scaled versions
of the volume, where each successive volume is half the size of the previous
one in all 3 dimensions.  This is mainly useful for the dds_save_volume
function.  The volume must have power of 2 size in all dimensions, and is
filtered with a BOX filter.]],

    { "param", "volume", "array of Images" },
    { "return", "array of arrays of Images" },
}

-- }}}

-- {{{ Text

doc { "function", "text_codepoint", module="Text",

[[Similar to the text() function but renders a single character (codepoint) of
text (provided as a UTF8 string).  The size of the returned image includes
spacing around the letter that is appropriate for collating this character with
others from the same font and size to form paragraphs of text.  The text() call
always removes blank pixels around the text so is not suitable for this
purpose.  Kerning information is not provided.]],

    { "param", "font", "string" },
    { "param", "size", "vector2" },
    { "param", "char", "string" },
    { "return", "Image" },
}

doc { "function", "text", module="Text",

[[Create a new image containing the rendered line of text.  The image is
automatically sized to fit the text.  Bitmap or scalable fonts can be used, and
are specified as a path e.g. to the ttf or the pcf.gz file.  The font size can
be chosen (width and height).  Optionally, a 2x2 matrix can be supplied, which
defines an affine transformation on the rendered text (rotation, scale, skew).
If given, this is applied after the scaling due to the font size.  The returned
grey scale image has the text rendered, antialiased, in white, on a black
background.  Combine this call with other features of luaimg to achieve drop
shadows, colour, etc.]],

    { "param", "font", "string" },
    { "param", "size", "vector2" },
    { "param", "text", "string" },
    { "param", "matrix_row1", "vector2", optional=true },
    { "param", "matrix_row2", "vector2", optional=true },
    { "return", "Image" },
}

-- }}}


-- {{{ Image Class

doc {
    "class",
    "Image",

[[A 2d rectangular grid of pixels.  Pixels are represented in single precision
floating point.  The image can have 1,2,3, or 4 colour channels.  If an image
has less than 4 channels, it is allowed to additionally have an alpha channel
(alpha channels have special behaviours when composing images).</p> <p> Images
can be combined by arithmetic (+,-,*,/,^).  The ..  operator combines images
according to alpha blending (i.e. regular blend mode in Photoshop/Gimp).  Other
blend modes are available via the mathematical operators.  You can therefore
mask images using the multiplication operator, add using the add operator, etc
(see examples above).</p>  <p>Individual pixel values of an image can be
accessed using the function call syntax, e.g. img(10,20).  Other functionality
is exposed via specific methods on images.  Images can be swizzled to extract
specific channels.  If a swizzle's last character is a capital letter, this
creates an alpha channel.  E.g. img.yX will yield a greyscale image with alpha
channel.  The value channel is the old green channel and the alpha channel is
the old red channel.  The two special swizzle characters f (full) and e (empty)
create a channel containing 1 or 0, respectively.]],

    { "field", "allChannels", "number", "The number of channels in the image (including alpha).", },
    { "field", "colourChannels", "number", "The number of channels in the image (not including alpha).", },
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
        "Create a new image with the given number of channels, the same size as this image, initialised by executing the function provided to map each pixel from this image into the new image.  The function is called with two params: the pixel from the existing image, and the coordinate being set (like make).",
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
        "Create a new image of the given size that is initialised to a copied version of this image, or the background colour if the pixel is not within the bounds of this image.  If no background colour is given, the image is wrapped (repeated).",
        { "param", "bottom_left", "vector2" },
        { "param", "size", "vector2" },
        { "param", "background", "colour", optional=true },
        { "return", "Image" },
    },
    {
        "method",
        "cropCentre",
        "As the crop method, except the location is fixed to the center.",
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
        "scaleBy",
        "Just like scale(), except the new size is given as a multiple of the old size.",
        { "param", "factor", {"vector2","number"} },
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
        "clamp",
        "The returned image's pixels are forced within the given range (inclusive).",
        { "param", "min", "colour" },
        { "param", "max", "colour" },
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
        "gamma",
        "Gamma encode/decode the given image.  Use a value of n > 1 to decode (usually 2.2) and 1/n to encode.  ",
        { "param", "n", "colour" },
        { "return", "Image" },
    },
    {
        "method",
        "quantise",
        "Reduce the colour fidelity of the image and also optionally dither it.  The available dither options are 'NONE', 'FLOYD_STEINBERG', and 'FLOYD_STEINBERG_LINEAR'.  FLOYD_STEINBERG assumes the image is in gamma space and temporarily converts the non-alpha channels to linear to do the dithering (you probably want this).  FLOYD_STEINBERG_LINEAR just does the dithering without that temporary conversion.  The number of colours is given as a vector with the same number of elements as the image has channels.  E.g. to reduce an RGB image to R5G6G5, use vec(32, 64, 32).  To use only 100% or 0% in each channel, use vec(2, 2, 2).",
        { "param", "dither", "string" },
        { "param", "num_colours", "vector" },
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
        "draw",
        "Assign a given colour to a single pixel coordinate.",
        { "param", "pos", "vector2" },
        { "param", "colour", "colour" },
    },
    {
        "method",
        "drawLine",
        "Draw a line between two points.",
        { "param", "start", "vector2" },
        { "param", "end", "vector2" },
        { "param", "width", "number" },
        { "param", "colour", "colour" },
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

-- }}}


function emit_html_file(name, content_func)

    file = io.open(name,"w")

    file:write(file_as_string("header.html"))

    file:write("    <div class='titleblock'>")
    file:write("        <a href='http://sourceforge.net/p/gritengine/code/HEAD/tree/trunk/luaimg/examples/logo.lua'><img class='logo' src='logo_large.png' alt='logo' /></a>")
    file:write("        <h1>LuaImg</h1>")
    file:write("        <div class='toplinks'>")
    file:write("            <a class='toplink' href='./'>Overview</a>")
    file:write("            <a class='toplink' href='examples.html'>Examples</a>")
    file:write("            <a class='toplink' href='download.html'>Download</a>")
    file:write("            <a class='toplink' href='usage.html'>Usage</a>")
    file:write("            <a class='toplink' href='api.html'>Script API</a>")
    file:write("        <div style='clear: both'></div>")
    file:write("        </div>")
    file:write("    </div>")

    content_func(file)

    if should_emit_validator then
        file:write('    <p style="float:right;"><a href="http://validator.w3.org/check?uri=referer"><img src="http://www.w3.org/Icons/valid-xhtml10" alt="Valid XHTML 1.0 Strict" height="31" width="88" /></a></p>')
    end

    if should_emit_analytics then
        file:write('<script type="text/javascript">')
        file:write('    (function(i,s,o,g,r,a,m){i["GoogleAnalyticsObject"]=r;i[r]=i[r]||function(){')
        file:write('    (i[r].q=i[r].q||[]).push(arguments)},i[r].l=1*new Date();a=s.createElement(o),')
        file:write('    m=s.getElementsByTagName(o)[0];a.async=1;a.src=g;m.parentNode.insertBefore(a,m)')
        file:write('    })(window,document,"script","//www.google-analytics.com/analytics.js","ga");')
        file:write('    ga("create", "UA-45157876-1", "gritengine.com");')
        file:write('    ga("send", "pageview");')
        file:write('</script>')
    end

    file:write(file_as_string("footer.html"))

    file:close()
end

emit_html_file("index.html", function (file) file:write(file_as_string("index_content.html")) end)
emit_html_file("examples.html", emit_examples)
emit_html_file("download.html", function (file) file:write(file_as_string("download_content.html")) end)
emit_html_file("usage.html", function (file) file:write(file_as_string("usage_content.html")) end)
emit_html_file("api.html", emit_api)


if should_emit_images then
        generate_imgs()
        include "../examples/logo.lua"
        include "../examples/lena_blueprint.lua"
        include "../examples/money.lua"
        include "../examples/bresenham_pattern.lua"
        include "../examples/aniblur.lua"
        include "../examples/anisharp.lua"
end
