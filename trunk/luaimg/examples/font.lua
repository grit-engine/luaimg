#!../luaimg.linux.x86_64 -F


all_codepoint_ranges = {
    {0x00020, 0x0007e}, -- basic latin
    {0x000a1, 0x000ff}, -- latin 1 supplement
    {0x00100, 0x0017f}, -- latin extended a
    {0x00300, 0x0036f}, -- combining diacritical marks
    {0x02010, 0x0205e}, -- general punctuation
    {0x0ff01, 0x0ffee}, -- halfwidth and fullwidth forms
    {0x0fffc, 0x0fffd}, -- undefined, error
     
    {0x02190, 0x021ff}, -- arrows
    {0x02580, 0x0259f}, -- block elements
    {0x02500, 0x0257f}, -- box drawing
    {0x020d0, 0x020f0}, -- combining diacritical marks for symbols
    {0x025a0, 0x025ff}, -- Geometric shapes
    {0x02200, 0x022ff}, -- mathematical operators
    {0x027c0, 0x027ef}, -- misc mathematical symbols A
    {0x02980, 0x029ff}, -- misc mathematical symbols B
    {0x02300, 0x023f3}, -- misc technical
    {0x02070, 0x0209f}, -- superscripts and subscripts
    {0x027f0, 0x027ff}, -- supplementary arrows a
    {0x02900, 0x0297f}, -- supplementary arrows b
    {0x02a00, 0x02aff}, -- supplementary math ops
--    {0x1d400, 0x1d7ff}, -- mathematical alphanumeric symbols NOT IN ORIGINAL FONT FOR SOME REASON
}

limited_codepoint_ranges = {
    {0x00020, 0x0007e}, -- basic latin
}
 
function make_font(tex_sz, font_sz, font, wide_font, tex_name, lua_name, codepoints, font_name)
        if wide_font == true then wide_font = font end

        new_tex = make(tex_sz, 1, false, 0)

        curr_x, curr_y = 0,0

        file = io.open(lua_name,"w")

        file:write("local codepoints = {\n")

        for _,range in ipairs(codepoints) do
            for cp=range[1],range[2] do
                local char = string.char(cp)
                local letter
                if char:getProperty("EAST_ASIAN_WIDTH") == "F" then
                        letter = text(wide_font, font_sz, char)
                else
                        letter = text(font, font_sz, char)
                end
                if curr_x + letter.width >= new_tex.width then
                    curr_y = curr_y + font_sz.y
                    curr_x = 0
                end
                new_tex:drawImage(letter.xX, vec(curr_x, curr_y))
                file:write(("    [0x%04x] = { %4d, %4d, %4d, %4d }, -- %s\n"):format(cp, curr_x, new_tex.height - curr_y - letter.height, letter.width, letter.height, char));
                curr_x = curr_x + letter.width
            end
        end


        file:write("}\n");
        file:write('gfx_font_define("'..font_name..'", "'..tex_name..'", '..tonumber(font_sz.y)..', codepoints)\n')

        file:close()

        new_tex.xX:save(tex_name)
end

make_font(vec(512,512), vec(6,13), "/usr/share/fonts/X11/misc/6x13.pcf.gz", "/usr/share/fonts/X11/misc/12x13ja.pcf.gz", "font_misc_fixed.png", "font_misc_fixed.lua", all_codepoint_ranges, "misc.fixed")

make_font(vec(512,256), vec(24,24),  "/usr/share/fonts/truetype/msttcorefonts/impact.ttf", true, "font_impact50.png", "font_impact50.lua", limited_codepoint_ranges, "Impact50")
make_font(vec(512,256), vec(24,24),  "/usr/share/fonts/truetype/msttcorefonts/impact.ttf", true, "font_impact24.png", "font_impact24.lua", limited_codepoint_ranges, "Impact24")
