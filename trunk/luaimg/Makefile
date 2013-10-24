# Makefile for building luaimg

ROOT = ..

include ../common.mk

LUA_A =  ../dependencies/lua-5.1.4/liblua.a

SOURCE = luaimg.cpp interpreter.cpp Image.cpp lua_wrappers_image.cpp
 
LUAIMG_CXXFLAGS = $(GRIT_CXX_CONFORMANCE) $(GRIT_CXX_CODEGEN) $(FREEIMAGE_CXXFLAGS) $(LUA_CXXFLAGS) -DLUA_USE_READLINE -Wno-type-limits
LUAIMG_LDFLAGS  = $(FREEIMAGE_LDFLAGS) $(LUA_LDFLAGS)
LUAIMG_LDLIBS   = $(FREEIMAGE_LDLIBS) $(LUA_LDLIBS) -lm -lreadline

luaimg: $(SOURCE) $(LUA_A)
	$(CXX) $(LUAIMG_CXXFLAGS) $(SOURCE) $(LUAIMG_LDFLAGS) $(LUAIMG_LDLIBS) -o $@

depend:
	makedepend -Y -f Makefile $(SOURCE) -I depend_stubs
	sed -i 's/^[a-zA-Z_]*[.]o:/luaimg:/g' Makefile

PACKAGED = \
    luaimg \
    luaimg.exe \
    doc/money.png \
    doc/lena_blueprint.png \
    doc/bresenham_pattern.png \
    doc/logo_large.png \
    doc/logo_med.png \
    doc/logo_small.png \
    doc/logo_tiny.png \
    doc/noise_hifreq.png \
    doc/perlin.png \
    doc/random.png \
    doc/red.png \
    doc/redb.png \
    doc/circle_bg_red.png \
    doc/circle.png \
    doc/circle_bg.png \
    doc/circle_a.png \
    doc/index.html \
    doc/examples.html \
    doc/download.html \
    doc/usage.html \
    doc/api.html \
    doc/doc.css \
    examples/lena.jpg \
    examples/bresenham_pattern.lua \
    examples/colour_map.lua \
    examples/colour_map_scurve.lua \
    examples/compass_ticks.lua \
    examples/font.lua \
    examples/gui_colourpicker.lua \
    examples/gui_slider_tex.lua \
    examples/lena_blueprint.lua \
    examples/logo.lua \
    examples/mandelbrot.lua \
    examples/money_input.png \
    examples/money.lua \
    examples/northern_lights.lua \
    examples/selftest.lua \
    examples/unblend.lua \

LI_PACKAGED = 

%.tar.bz2: $(PACKAGED)
	ln -sf . $(shell basename $@ .tar.bz2)
	tar  -cjvf $@ $(patsubst %,$(shell basename $@ .tar.bz2)/%,$(PACKAGED))

%.zip: $(PACKAGED)
	zip $@ $(PACKAGED)

clean:
	rm -fv luaimg

# DO NOT DELETE

luaimg: depend_stubs/cstdlib depend_stubs/string depend_stubs/vector
luaimg: depend_stubs/sstream depend_stubs/iostream depend_stubs/FreeImage.h
luaimg: depend_stubs/lua.h depend_stubs/lauxlib.h depend_stubs/lualib.h
luaimg: interpreter.h Image.h depend_stubs/cinttypes depend_stubs/cmath
luaimg: depend_stubs/ostream
luaimg: depend_stubs/cstdlib depend_stubs/string depend_stubs/vector
luaimg: depend_stubs/iostream depend_stubs/signal.h depend_stubs/lua.h
luaimg: depend_stubs/lauxlib.h depend_stubs/lualib.h interpreter.h
luaimg: lua_wrappers_image.h Image.h depend_stubs/cinttypes
luaimg: depend_stubs/cmath depend_stubs/ostream
luaimg: depend_stubs/cmath depend_stubs/string depend_stubs/iostream
luaimg: depend_stubs/FreeImage.h Image.h depend_stubs/cinttypes
luaimg: depend_stubs/ostream
luaimg: depend_stubs/iostream depend_stubs/string
luaimg: depend_stubs/sstream depend_stubs/vector
luaimg: depend_stubs/limits depend_stubs/algorithm
luaimg: depend_stubs/lua.h depend_stubs/lauxlib.h
luaimg: depend_stubs/lualib.h lua_wrappers_image.h Image.h
luaimg: depend_stubs/cinttypes depend_stubs/cmath
luaimg: depend_stubs/ostream

# vim: noet:ts=8:sw=8
