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

[[make(vec(64,64), 3, vec(1,0,0)):save("red.png")]],

[[local sz = vec(64,64)
function init(pos)
    local rad = #(pos - sz/2);
    local alpha = clamp(30-rad, 0, 1)
    return vec(0, 0, 1, alpha)
end
make(sz, 4, init):save("circle.png")]],

[[(open("circle.png") ^ vec(1,1,0)):save("circle_bg.png")]],

[[(open("circle.png") ^ open("red.png")):save("circle_bg2.png")]],

[[open("circle.png").w:save("circle_a.png")]],

[[make(vec(64,64), 3, function() return vec(random(), random(), random()) end):save("random.png")]],

[[open("random.png"):convolveSep(gaussian(7), true, true):save("perlin.png")]],

[[((vec(1,0,1)*open("random.png")+open("perlin.png"))/2):save("arith.png")]]

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

        <p>LuaImg is a programming framework for scripted image manipulation.  It does not compete with Gimp/Photoshop since those applications are for interactive image manipulation.  It is more similar to ImageMagick and netpbm.  However, those tools encourage the use of the shell to glue everything together.  LuaImg is more like sed or awk, in that a single LuaImg process can do an arbitrary amount of manipulation using an internal domain-specific language.  LuaImg is based on the Lua fork used in the <a href="http://www.gritengine.com/">Grit</a> project.  The goals of LuaImg are, in order:</p>
        <ol>
            <li>Expressive power: To allow the expression of a wide variety of image manipulation algorithms on HDR and LDR images.</li>
            <li>Simplicity:  To provide the smallest set of features, allow complicated effects to be achieved through easy and intuitive combinations of these features.</li>
            <li>Performance:  To provide the best performance possible without compromising goals 1 and 2.  The use of script interpretion does limit performance, but we try to keep the garbage collection costs under control with a customised virtual machine.</li>
        </ol>

        <p>LuaImg has so far been used for detecting northern lights in photos
of the sky, preparing bitmap font textures for computer games, attempting to
undo blend operations, and more.</p>

    </div>

    <h2>Types</h2>

    <div class="prose">

        <p>LuaImg has all the types from regular Lua (number, boolean, string, function, table, and the nil type).</p>

        <p>In addition, it has a number of compound value types.  This is
important because they  do not incur garbage collection overhead, just like
number, boolean, and nil.  These types are vector2, vector3, and vector4, which
are useful for representing coordinates and colours.  (There is also a
quaternion type which is arguably less useful in this context.)  These values
are created using constructors, e.g., vector2(10,20) or vector4(1,2,3,4).
Individual fields can be accessed using .x, .y, .z, or .w field accesses (the
fields can be read but not written).  It is also possible to swizzle, e.g.,
.xyz will produce a vector3, .xz will produce a vector2, etc.</p>

        <p>Arithmetic operations on the vector types operate in a point-wise
manner, e.g.  vector3(1,2,3) * vector3(-1,2,1) = vector3(-1,4,3).  The #
operator on a vector provides its length (Euclidian length, i.e. Pythagoras).
Some of the standard lua math functions (pow, max, min, clamp) have been
adapted to handle vector values, but the rest still operate on number only.
Additional functions like dot product, etc, are described below. </p>

    </div>

    <h2>Other differences from Lua</h2>

    <div class="prose">
        <p>All of the math functions have been moved out of the math package into global scope.</p>
    </div>

    <h2>Examples</h2>

    <div class="prose">

        <p>Lua programs are also LuaImg programs, so the Lua hello world program stands:</p>
        <pre>
print "Hello world!" </pre>

        <p>Create a 64x64 image with 3 channels containing solid red (vector3(1,0,0)).  Save to a file:</p>
        <img src="red.png" style="float:right;" alt="luaimg output"/>
        <pre>]]..examples[1]..[[</pre>

        <p>Initialising an image with a function (draws an antialiased blue circle with alpha):</p>
        <img src="circle.png" style="float:right;" alt="luaimg output" />
        <pre>]]..examples[2]..[[</pre>

        <p>Blend the previous image onto a solid yellow background:</p>
        <img src="circle_bg.png" style="float:right;" alt="luaimg output" />
        <pre>]]..examples[3]..[[</pre>

        <p>Blend the previous image onto red.png:</p>
        <img src="circle_bg2.png" style="float:right;" alt="luaimg output" />
        <pre>]]..examples[4]..[[</pre>

        <p>Load previous image, extract alpha channel and save it:</p>
        <img src="circle_a.png" style="float:right;" alt="luaimg output" />
        <pre>]]..examples[5]..[[</pre>

        <p>Random noise:</p>
        <img src="random.png" style="float:right;" alt="luaimg output" />
        <pre>]]..examples[6]..[[</pre>

        <p>Random noise with gaussian blur.  The gaussian(n) function returns an nx1 image that represents a separated normalised gaussian blur kernel.  Custom kernels, whether separable or not, are easy in LuaImg.</p>
        <img src="perlin.png" style="float:right;" alt="luaimg output" />
        <pre>]]..examples[7]..[[</pre>

        <p>Arithmetic on images:</p>
        <img src="arith.png" style="float:right;" alt="luaimg output" />
        <pre>]]..examples[8]..[[</pre>

        <p>Note that with any programming framework, the point is not to do simple
    one-off things like in these examples, but to build complex, precise, or
    repetitive things that are not possible except through programming.</p>

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
        and number of channels.</li>
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
    channels.  The init parameter can be either a single colour (for a solid
    image), an array of colours of size W*H, or a function that returns the colour
    at each pixel.]],
    { "param", "size", "vector2" },
    { "param", "channels", {1,2,3,4} },
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
    [[A 2d rectangular grid of pixels.  Pixels are represented in single precision floating point.  The image can have 1,2,3, or 4 channels.
      Images can be manipulated by ordinary arithmetic and are garbage collected.  You can therefore mask images using the multiplication operator, etc (see provided examples).
      Individual pixel values can be accessed using the call syntax img(10,20).  Other functionality is exposed via methods.  ]],
    { "field", "channels", "number", "The number of channels in the image.", },
    { "field", "width", "number", "The number of pixels in a row of the image.", },
    { "field", "height", "number", "The number of pixels in a column of the image.", },
    { "field", "size", "vector2", "The width and height as a single value.", },
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
        "Create a new image with the given number of channels, the same size as this image, initailised by executing the function provided to map each pixel from this image into the new image.  The function is called with both the colour from the existing image, and the coordinate being set (like make).",
        { "param", "channels", "number" },
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
        "rms",
        "Subtract one image from the other.  Square every pixel value.  Sum all the pixels, and square root the result.  This is a common method for objectively measuring the difference between two compatible images.",
        { "param", "other", "Image" },
        { "return", "number" },
    },
    {
        "method",
        "pow",
        "Return a new image with each pixel raised to the given power.  This is useful for gamma conversions, e.g. to decode a gamma-encoded image, use img:pow(2.2).",
        { "param", "index", "number" },
        { "return", "Image" },
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
        "lerp",
        "The returned image is the lerp of the two given images according to the given alpha value.  It is also allowed to give a single colour value in place of the other image.",
        { "param", "other", "Image/colour" },
        { "param", "alpha", "number" },
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
