#!../luaimg.linux.x86_64 -F

-- Generates a wrapped texture of bathroom floor tiles in a 'knot' pattern, i.e. a pattern that
-- gives the impression of an overlapping weave of materials.

local grout_colour = 0.25
local big_tile_colour = .95
local small_tile_colour = vec(0, 0, .8)

local big_tile_sz = vec(200, 126)
local grout_sz = 4
local tex_sz = vec(1, 1) * (big_tile_sz.x + grout_sz + big_tile_sz.y + grout_sz)
local small_tile_sz = vec(1, 1) * (tex_sz.x - 2*big_tile_sz.y - 4*grout_sz)/2

local big_tile_h = make(big_tile_sz, 3, true, big_tile_colour)
local big_tile_v = make(big_tile_sz.yx, 3, true, big_tile_colour)
local small_tile = make(small_tile_sz, 3, true, small_tile_colour)

local tex = make(tex_sz, 3, grout_colour)

tex:drawImageAt(big_tile_h, vec(0, 0), true, true)
tex:drawImageAt(big_tile_v, vec(tex_sz.x/2, 0), true, true)
tex:drawImageAt(big_tile_v, vec(0, tex_sz.y/2), true, true)
tex:drawImageAt(big_tile_h, vec(tex_sz.x/2, tex_sz.y/2), true, true)

tex:drawImageAt(small_tile, tex_sz*0.25, true, true)
tex:drawImageAt(small_tile, tex_sz*vec(0.25, 0.75), true, true)
tex:drawImageAt(small_tile, tex_sz*0.75, true, true)
tex:drawImageAt(small_tile, tex_sz*vec(0.75, 0.25), true, true)

-- wrap up to 4x size
tex:cropCentre(tex.size*4):save("tiled_knot.png")
