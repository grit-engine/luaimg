old_width = 0
old_height = 0
old_codepoints = nil
old_tex = open("old_font.png").w

function add_font(name,texture,tw,th,codepoints)
    old_width = tw
    old_height = th
    old_codepoints = codepoints
end

include "../../grit_core/media/system/misc.fixed.lua"

desired_codepoint_ranges = {
    {0x00021, 0x0007e}, -- basic latin
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
--  {0x1d400, 0x1d7ff}, -- mathematical alphanumeric symbols
    {0x02200, 0x022ff}, -- mathematical operators
    {0x027c0, 0x027ef}, -- misc mathematical symbols A
    {0x02980, 0x029ff}, -- misc mathematical symbols B
    {0x02300, 0x023f3}, -- misc technical
    {0x02070, 0x0209f}, -- superscripts and subscripts
    {0x027f0, 0x027ff}, -- supplementary arrows a
    {0x02900, 0x0297f}, -- supplementary arrows b
    {0x02a00, 0x02aff}, -- supplementary math ops
}
 
-- convert dds to png
-- list of codepoints we want

new_tex = make(vec(512, 512), 2, true, 0)
new_codepoints = { }

curr_x, curr_y = 0,0

kernel = gaussian(3)*1.5

for _,range in ipairs(desired_codepoint_ranges) do
    for cp=range[1],range[2] do
        local old_x, old_y, w, h = unpack(old_codepoints[cp])
        local letter = old_tex:crop(vec(old_x, old_tex.height-old_y-h), vec(w,h))
        local shadow = letter:convolveSep(kernel);
        if curr_x + w >= new_tex.width then
            curr_y = curr_y + h
            curr_x = 0
        else
        end
        new_tex:drawImage(0*shadow.xX, vec(curr_x, curr_y))
        new_tex:drawImage(letter.xX, vec(curr_x, curr_y))
        new_codepoints[cp] = { curr_x, curr_y, w, h }
        curr_x = curr_x + w
    end
end

print ("codepoints = {")
for k=0,2^16 do
    local tab = new_codepoints[k]
    if tab ~= nil then
        local x, y, w, h = tab[1], tab[2], tab[3], tab[4]
        y = new_tex.height - y - h
        print(("    [0x%04x] = { %3d, %3d, %3d, %3d },"):format(k, x, y, w, h));
    end
end
print ("}");

(new_tex.xxxY..vec(0,0,1)):save("new_font.png")
