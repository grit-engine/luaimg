local docs = {
    functions = {},
    classes = {}
}

function doc_func(tab, entry)
    entry.params = {}
    entry.returns = {}
    for k=4,#tab do
        if tab[k][1] == "param" then
            local dest = entry.params
            dest[#dest+1] = { name = tab[k][2], kind = tab[k][3], optional = tab[k].optional }
        elseif tab[k][1] == "return" then
            local dest = entry.returns
            dest[#dest+1] = { kind = tab[k][2] }
        end
    end
end

function doc(tab)
    local kind=tab[1]
    local name=tab[2]
    local entry = {
        desc = tab[3],
    }
    if kind == "function" then
        doc_func(tab, entry)
        docs.functions[name] = entry
    elseif kind == "class" then
        entry.fields = {}
        entry.methods = {}
        for k=4,#tab do
            local n = tab[k][2]
            if tab[k][1] == "field" then
                entry.fields[n] = { kind = tab[k][3], desc=tab[k][4] }
            elseif tab[k][1] == "method" then
                local meth = { desc=tab[k][3] }
                doc_func(tab[k], meth)
                entry.methods[n] = meth
            end
        end
        docs.classes[name] = entry
    end
end



local function sorted_keys_from(tab)
    local keys = {}
    for name,_ in pairs(tab) do
        keys[#keys+1] = name
    end
    table.sort(keys)
    return keys
end

local function concat(...)
    local content = ""
    for i=1,select("#",...) do
        content = content..select(i,...)
    end
    return content
end

local examples = {

[[my_img = make(vec(32,32), 3, vec(1,0,0))
my_img:save("red.png")]],

[[local sz = vec(64,64)
function init(pos)
    local rad = #(pos - sz/2);
    local alpha = clamp(30-rad, 0, 1)
    return vec(0, 0, 1, alpha)
end
circle = make(sz, 4, true, init)
circle:save("circle.png")]],

[[(circle .. vec(1,1,0)):save("circle_bg.png")]],

[[smaller = circle:scale(vec(32,32),"BICUBIC")
blended = smaller .. open("red.png")
blended:save("circle_bg2.png")]],

[[my_mask = circle.w
my_mask:save("circle_a.png")]],

[[
function randvec() return vec(random(), random(), random()) end
my_noise = make(vec(64,64), 3, randvec)
my_noise:save("random.png")]],

[[
my_perlin = my_noise:convolveSep(gaussian(7), true, true)
my_perlin:save("perlin.png")]],

[[((my_noise-my_perlin) + 0.5):save("noise_hifreq.png")]]

}

function generate_imgs()

    for _,v in ipairs(examples) do
        loadstring(v)()
    end

end

function emit_page()
    print ([[<!DOCTYPE html PUBLIC "-//W3C//DTD XHTML 1.0 Strict//EN" "http://www.w3.org/TR/xhtml1/DTD/xhtml1-strict.dtd">
<html xmlns="http://www.w3.org/1999/xhtml">
<head>
    <title>LuaImg Documentation</title>
    <meta http-equiv="Content-type" content="text/html;charset=UTF-8" />
    <link rel="stylesheet" type="text/css" href="doc.css" />
</head>
<body>

    <h1>LuaImg Documentation</h1>

    <h2>Overview</h2>

    <div class="prose">

        <p>LuaImg is a programming framework for scripted (i.e.
non-interactive) image manipulation.  The images are stored in single precision
floating point, and manipulated with a high-level scripting language based on
the Lua fork from the <a href="http://www.gritengine.com/">Grit</a> project.
Intermediate images are garbage-collected instead of being explicitly re-used.
This allow complex mathematical algorithms to be written in a natural way.
While these niceties come with a performance cost, the productivity gains more
than make up for it, especially for rapid prototyping or experimentation with
images.  Having said this, performance is generally quite good.  Large images
such as the 5760x3840 resolution images created by the Canon 5DmkIII can be
processed pixel-by-pixel with custom Lua code in a few seconds.</p>

<p> LuaImg is similar to ImageMagick and netpbm.  However, those tools
encourage the use of shell programming to glue everything together.  They are
also somewhat limited in their programmability. LuaImg is more like sed or awk,
in that a single LuaImg process can do an arbitrary amount of manipulation
using an internal domain-specific language.  LuaImg shares some idioms with
shader programming, although all code runs on the CPU.  The goals of LuaImg
are, in order:</p>

        <ol>
            <li>Expressive power: To allow the expression of a wide variety of image manipulation algorithms on HDR images.</li>
            <li>Simplicity:  To provide the smallest set of features, allow complicated effects to be achieved through easy and intuitive combinations of these features.</li>
            <li>Performance:  To provide the best performance possible without compromising goals 1 and 2.  The use of script interpretion does limit performance, but we try to keep the garbage collection costs under control with a customised virtual machine.</li>
        </ol>

        <p>Note that with any programming framework, the point is not to do simple
    one-off things, but to perform complex, precise, or
    repetitive tasks that are difficult or time-consuming without programming.</p>

        <p>LuaImg has so far been used for detecting northern lights in photos
of the sky, preparing bitmap font textures for computer games, extracting alpha
masks from blended images, and more.</p>

    </div>

    <h2>Types</h2>

    <div class="prose">

        <p>LuaImg has all the types from regular Lua (number, boolean, string, function, table, and the nil type).</p>

        <p>In addition, it has a number of compound value types.  This is
important because they  do not incur garbage collection overhead, just like
number, boolean, and nil.  These types are vector2, vector3, and vector4, which
are useful for representing coordinates and colours.  (There is also a
quaternion type which is arguably less useful in this context.)  These values
are created using constructors, e.g., vec(10,20) or vec(1,2,3,4).
Individual fields can be accessed using .x, .y, .z, or .w field accesses (the
fields can be read but not written).  It is also possible to swizzle, e.g.,
.xyz will produce a vector3, .xz will produce a vector2, etc.</p>

        <p>Arithmetic operations on the vector types operate in a point-wise
manner, e.g.  vec(1,2,3) * vec(-1,2,1) = vec(-1,4,3).  The #
operator on a vector provides its length (Euclidian length, i.e. Pythagoras).
</p>

    </div>

    <h2>Other differences from Lua</h2>

    <div class="prose">

        <p>All of the math functions have been moved out of the math package
into global scope.Some of the standard Lua math functions (pow, ceil, floor,
abs, clamp) have been adapted to also support vector values, but the rest still
operate on number only.  The unpack function will convert vectors to their
components. Additional functions like dot product, etc, are described below.
</p>

    </div>

    <h2>Examples</h2>

    <div class="prose">

        <p>Lua programs are also LuaImg programs, so the Lua hello world program stands:</p>
        <pre>
print "Hello world!" </pre>

        <p>Create a 32x32 image with 3 channels containing solid red (vec(1,0,0)).  Save to a file (can also be written on one line):</p>
        <img src="red.png" style="float:right;" alt="LuaImg output"/>
        <pre>]]..examples[1]..[[</pre>

        <p>Initialising a 3 channel image with alpha, initialised with a function (computes an antialiased blue circle with alpha):</p>
        <img src="circle.png" style="float:right;" alt="LuaImg output" />
        <pre>]]..examples[2]..[[</pre>

        <p>Blend the circle image onto a solid yellow background:</p>
        <img src="circle_bg.png" style="float:right;" alt="LuaImg output" />
        <pre>]]..examples[3]..[[</pre>

        <p>Blend the circle image onto red.png:</p>
        <img src="circle_bg2.png" style="float:right;" alt="LuaImg output" />
        <pre>]]..examples[4]..[[</pre>

        <p>Extract alpha channel and save it:</p>
        <img src="circle_a.png" style="float:right;" alt="LuaImg output" />
        <pre>]]..examples[5]..[[</pre>

        <p>Random noise:</p>
        <img src="random.png" style="float:right;" alt="LuaImg output" />
        <pre>]]..examples[6]..[[</pre>

        <p>Random noise with gaussian blur.  The gaussian(n) function returns an nx1 image that represents a separated normalised gaussian blur kernel.  One can also use custom kernels by providing an image instead of using the result of gaussian() -- and these can be provided either in separated form, or as a general rectangular matrix.</p>
        <img src="perlin.png" style="float:right;" alt="LuaImg output" />
        <pre>]]..examples[7]..[[</pre>

        <p>Subtract the two to create a high frequency noise texture.  Generally, using arithmetic to combine images is supported.</p>
        <img src="noise_hifreq.png" style="float:right;" alt="LuaImg output" />
        <pre>]]..examples[8]..[[</pre>

    </div>

    <h2>API notation conventions</h2>

    <div class="prose">

        <p>The required types are written in the documentation, but with a few
        ingenuities:</p>
        <ul>
            <li>  A <i>colour</i> can be a number, vector2, vector3, or vector4
        depending on how many channels are required (this should be clear from the
        context).</li>
            <li>Images are said to be <i>compatible</i> if they have the same size
        and number of channels, or one of them has only a single channel (a mask).  Number/vector values can be used in place of an image, in which case they act like a solid colour image.</li>
            <li>An <i>array</i> is defined to be a Lua table
        containing only number keys between 1 and some other number (without any gaps).</li>
            <li> The type of a function, when passed as a value, is denoted in the API
        documentation with an arrow, e.g., math.sqrt() would be described as
        (number)->(number).</li>
            <li>Optional parameters are given in square brackets.</li>
        </ul>
    </div>
]])

    print '    <h2>Global Functions</h2>'
    for _,name in ipairs(sorted_keys_from(docs.functions)) do
        print(translate_func(name,docs.functions[name]))
    end

    print '    <h2>Classes</h2>'
    for _,name in ipairs(sorted_keys_from(docs.classes)) do
        print(translate_class(name,docs.classes[name]))
    end

    print [[    <p style="float:right;"><a href="http://validator.w3.org/check?uri=referer"><img src="http://www.w3.org/Icons/valid-xhtml10" alt="Valid XHTML 1.0 Strict" height="31" width="88" /></a></p>
</body>
</html>
]]
end

local function span(class, ...)
    return "<span class=\""..class.."\">"..concat(...).."</span>"
end
local function div(class, ...)
    return "<div class=\""..class.."\">"..concat(...).."</div>"
end
function para(p)
    return "<p>"..p.."</p>"
end

-- Different whitespace variations
local function spanline(indent, class, ...)
    return indent.."<span class=\""..class.."\">"..concat(...).."</span>".."\n"
end
local function divline(indent, class, ...)
    return indent.."<div class=\""..class.."\">"..concat(...).."</div>".."\n"
end
local function divblk(indent, class, ...)
    return indent.."<div class=\""..class.."\">\n"..concat(...)..indent.."</div>\n"
end


function translate_type(kind)
    if type(kind) == "table" then
        local r = ""
        local sep = "{ "
        for _,v in ipairs(kind) do
            r = r..sep..tostring(v)
            sep = ", "
        end
        return r.." }"
    else
        return kind
    end
end

function translate_func_body(indent, nameclass, descclass, name, entry)
    local paramlist = ""
    local endline = #entry.params>2 and " <br/>" or ""
    for k,v in ipairs(entry.params) do
        local before, after = "", ""
        if v.optional then before, after = "[", "]" end
        paramlist = paramlist .. before ..  span("paramname", v.name)
                              .. span("paramtype", " : "..translate_type(v.kind)) .. after
                              .. (k==#entry.params and "" or span("paramparen",",")
                              .. endline)
    end
    paramlist = divline(indent.."    ","paramlist", paramlist..span("paramparen",")"))

    local returnlist = ""
    if #entry.returns ~= 0 then
        if #entry.returns > 2 then
            for k,v in ipairs(entry.returns) do
                returnlist = returnlist .. span("paramtype", translate_type(v.kind))
                                        .. (k==#entry.returns and "" or span("returncomma",", ").."<br/>")
            end
        else
            for k,v in ipairs(entry.returns) do
                returnlist = returnlist .. span("paramtype", translate_type(v.kind))
                                        .. (k==#entry.returns and "" or span("returncomma",", ")) 
            end
        end
        returnlist = divline(indent.."    ","returnlist", returnlist)
        returnlist = spanline(indent.."    ","returns", "Returns:")..returnlist
        returnlist = divblk(indent, "returns_sec", returnlist)
    end

    return divblk(indent, "signature",
                divline(indent.."    ", nameclass, name, span("paramparen","(")),
                paramlist
            ),
            divline(indent, descclass, para(entry.desc)),
            returnlist
end

function translate_func(name, entry)

    return divblk("    ","function",
            divline("    ","functionhead", name, " (global function)"),
            divblk("    ","functionbody", translate_func_body("        ", "name", "desc", name, entry))
        )
end

function translate_class(name, entry)
    local fieldlist = ""
    local fieldkeys = sorted_keys_from(entry.fields)
    if #fieldkeys ~= 0 then
        for _,k in ipairs(fieldkeys) do
            local v = entry.fields[k]
            fieldlist = fieldlist .. divline("                ","classfield",
                                                span("classfieldname", k),
                                                span("paramtype", " : "..translate_type(v.kind)),
                                                div("classfielddesc", para(v.desc))
                                            )
        end
        fieldlist = divblk("            ","classfieldlist", fieldlist)
        fieldlist = divline("            ","classdivider", "Fields:")..fieldlist
    end


    local methodlist = ""
    local methodkeys = sorted_keys_from(entry.methods)
    if #methodkeys ~= 0 then
        for _,k in ipairs(methodkeys) do
            local v = entry.methods[k]
            methodlist = methodlist .. divblk("                ","classmethod",translate_func_body("                    ", "classmethodname", "classmethoddesc", k, v))
        end
        methodlist = divblk("            ","classmethodlist", methodlist)
        methodlist = divline("            ","classdivider", "Methods:")..methodlist
    end

    return divblk("    ","class",
        divline("        ","classhead",name.." (class)"),
        divblk("        ","classbody",
            divline("            ","desc", para(entry.desc)),
            fieldlist,
            methodlist
        )
    )
end

doc {
    "function",
    "make",
    [[Create a new image of the specificed size, with the specified number of
    channels (alpha defaults to false).  The init parameter can be either a single colour (for a solid
    image), an array of colours of size W*H, or a function that returns the colour
    at each pixel.]],
    { "param", "size", "vector2" },
    { "param", "channels", {1,2,3,4} },
    { "param", "alpha", "boolean", optional=true },
    { "param", "init", {"colour", "array[colour]",  "(vector2)->(colour)"} },
    { "return", "Image" },
}
    
doc {
    "function",
    "open",
    "Load an image file from disk.",
    { "param", "filename", "string" },
    { "return", "Image" },
}

doc {
    "function",
    "lerp",
    "Interpolate between two colours / images.  T can be number, vector2/3/4, or Image.  If lerping images, they must be compatible.",
    { "param", "v1", "T" },
    { "param", "v2", "T" },
    { "param", "alpha", "number" },
    { "return", "T" },
}

doc {
    "function",
    "gaussian",
    "Generate a separated Gaussian convolution kernel.  This is an nx1 image containing that row of Pascal's triangle, normalised so it all sums to 1.",
    { "param", "n", "number" },
    { "return", "Image" },
}

doc {
    "function",
    "colour",
    "Return a vector value of the given dimensionality, all of whose elements are the given value.",
    { "param", "d", "number" },
    { "param", "n", "number" },
    { "return", "vector" },
}

doc {
    "function",
    "dot",
    "Compute the dot product of the given vectors.",
    { "param", "a", "vector" },
    { "param", "b", "vector" },
    { "return", "number" },
}

doc {
    "function",
    "cross",
    "Compute the cross product of the given vectors.",
    { "param", "a", "vector3" },
    { "param", "b", "vector3" },
    { "return", "vector3" },
}

doc {
    "function",
    "inv",
    "Invert a quaternion.",
    { "param", "a", "quat" },
    { "return", "quat" },
}

doc {
    "function",
    "slerp",
    "Interpolate between two quaternions.",
    { "param", "a", "quat" },
    { "param", "b", "quat" },
    { "param", "alpha", "number" },
    { "return", "quat" },
}

doc {
    "function",
    "norm",
    "Normalise the vector or quaternion (return a value that has length 1 but is otherwise equivalent).",
    { "param", "a", {"vector","quat"} },
    { "return", {"vector","quat"} },
}

acros = { "HSL", "HSV", "RGB" }
exploded = { "hue/saturation/luminance", "hue/saturation/value", "red/green/blue" }
for _,pair in ipairs{{1,2},{2,1},{1,3},{3,1},{2,3},{3,2}} do
    doc {
        "function",
        acros[pair[1]].."to"..acros[pair[2]],
        "Convert "..exploded[pair[1]].." to "..exploded[pair[2]]..".",
        { "param", "colour", "vector3" },
        { "return", "vector3" },
    }
end

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

    { "field", "channels", "number", "The number of channels in the image.", },
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
        "Modify this image by drawing the other image onto it, in the given position.  If the other image has one more channel than this one, then alpha blend it using that alpha channel.  Otherwise the images must have the same number of channels, and the last channel is assumed to be an alpha channel in each case.",
        { "param", "other", "Image" },
        { "param", "bottom_left", "vector2" },
    },
}

emit_page()
generate_imgs()
