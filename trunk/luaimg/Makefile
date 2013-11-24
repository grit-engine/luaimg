# Makefile for building luaimg

ROOT = ..

include ../common.mk

LUA_A =  ../dependencies/lua-5.1.4/liblua.a

SOURCE = luaimg.cpp interpreter.cpp Image.cpp lua_wrappers_image.cpp text.cpp ../dependencies/util/lua_utf8.cpp  ../dependencies/util/lua_util.cpp  ../dependencies/util/unicode_util.cpp ../dependencies/util/console.cpp
 
LUAIMG_CXXFLAGS = $(ICU_CXXFLAGS) $(GRIT_CXX_CONFORMANCE) $(GRIT_CXX_CODEGEN) $(FREEIMAGE_CXXFLAGS) $(LUA_CXXFLAGS) -DLUA_USE_READLINE -Wno-type-limits $(shell pkg-config freetype2 --cflags) -I ../dependencies/util
LUAIMG_LDFLAGS  = $(ICU_LDFLAGS) $(FREEIMAGE_LDFLAGS) $(LUA_LDFLAGS) $(shell pkg-config freetype2 --libs-only-L)
LUAIMG_LDLIBS   = $(ICU_LDLIBS) $(FREEIMAGE_LDLIBS) $(LUA_LDLIBS) -lm -lreadline $(shell pkg-config freetype2 --libs-only-l)

luaimg.$(GRIT_EXEC_SUFFIX): $(SOURCE) $(LUA_A) $(ICU_ARCHIVES)
	$(CXX) $(LUAIMG_CXXFLAGS) $(SOURCE) $(LUAIMG_LDFLAGS) $(LUAIMG_LDLIBS) -o $@

depend:
	makedepend -Y -f Makefile $(SOURCE) -I../dependencies/util -I depend_stubs
	sed -i "s/^[a-zA-Z_]*[.]o:/luaimg.$(GRIT_EXEC_SUFFIX):/g" Makefile

PACKAGED = \
    luaimg.linux.x86_64 \
    luaimg.exe \
    icudt42.dll \
    icuin42.dll \
    icuuc42.dll \
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
    examples/compass_rose.lua \
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

luaimg.linux.x86_64: depend_stubs/cstdlib depend_stubs/string depend_stubs/vector
luaimg.linux.x86_64: depend_stubs/sstream depend_stubs/iostream depend_stubs/FreeImage.h
luaimg.linux.x86_64: depend_stubs/lua.h depend_stubs/lauxlib.h depend_stubs/lualib.h
luaimg.linux.x86_64: interpreter.h Image.h depend_stubs/cinttypes depend_stubs/cmath
luaimg.linux.x86_64: depend_stubs/cassert depend_stubs/ostream text.h
luaimg.linux.x86_64: depend_stubs/ft2build.h
luaimg.linux.x86_64: depend_stubs/cstdlib depend_stubs/string depend_stubs/vector
luaimg.linux.x86_64: depend_stubs/iostream depend_stubs/signal.h depend_stubs/lua.h
luaimg.linux.x86_64: depend_stubs/lauxlib.h depend_stubs/lualib.h
luaimg.linux.x86_64: ../dependencies/util/lua_util.h depend_stubs/sstream
luaimg.linux.x86_64: depend_stubs/limits ../dependencies/util/math_util.h
luaimg.linux.x86_64: depend_stubs/cmath depend_stubs/cfloat
luaimg.linux.x86_64: ../dependencies/util/lua_utf8.h ../dependencies/util/console.h
luaimg.linux.x86_64: interpreter.h lua_wrappers_image.h Image.h
luaimg.linux.x86_64: depend_stubs/cinttypes depend_stubs/cassert
luaimg.linux.x86_64: depend_stubs/ostream
luaimg.linux.x86_64: depend_stubs/cstdlib depend_stubs/cstdio depend_stubs/cmath
luaimg.linux.x86_64: depend_stubs/cstring depend_stubs/string depend_stubs/iostream
luaimg.linux.x86_64: depend_stubs/fstream depend_stubs/FreeImage.h Image.h
luaimg.linux.x86_64: depend_stubs/cinttypes depend_stubs/cassert depend_stubs/ostream
luaimg.linux.x86_64: depend_stubs/iostream depend_stubs/string
luaimg.linux.x86_64: depend_stubs/sstream depend_stubs/vector
luaimg.linux.x86_64: depend_stubs/limits depend_stubs/algorithm
luaimg.linux.x86_64: depend_stubs/lua.h depend_stubs/lauxlib.h
luaimg.linux.x86_64: depend_stubs/lualib.h ../dependencies/util/lua_util.h
luaimg.linux.x86_64: ../dependencies/util/math_util.h depend_stubs/cmath
luaimg.linux.x86_64: depend_stubs/cfloat lua_wrappers_image.h Image.h
luaimg.linux.x86_64: depend_stubs/cinttypes depend_stubs/cassert
luaimg.linux.x86_64: depend_stubs/ostream text.h depend_stubs/ft2build.h
luaimg.linux.x86_64: depend_stubs/cstdlib depend_stubs/cstdio depend_stubs/cmath
luaimg.linux.x86_64: depend_stubs/cstring depend_stubs/string depend_stubs/iostream
luaimg.linux.x86_64: depend_stubs/fstream ../dependencies/util/lua_util.h
luaimg.linux.x86_64: depend_stubs/sstream depend_stubs/limits depend_stubs/vector
luaimg.linux.x86_64: depend_stubs/lua.h depend_stubs/lauxlib.h depend_stubs/lualib.h
luaimg.linux.x86_64: ../dependencies/util/math_util.h depend_stubs/cfloat
luaimg.linux.x86_64: ../dependencies/util/unicode_util.h text.h depend_stubs/ostream
luaimg.linux.x86_64: depend_stubs/ft2build.h Image.h depend_stubs/cinttypes
luaimg.linux.x86_64: depend_stubs/cassert
../dependencies/util/lua_utf8.o: depend_stubs/cstdlib depend_stubs/cstdio
../dependencies/util/lua_utf8.o: depend_stubs/cassert depend_stubs/iostream
../dependencies/util/lua_utf8.o: depend_stubs/string depend_stubs/sstream
../dependencies/util/lua_utf8.o: depend_stubs/lua.h
../dependencies/util/lua_utf8.o: depend_stubs/unicode/unistr.h
../dependencies/util/lua_utf8.o: depend_stubs/unicode/uchar.h
../dependencies/util/lua_utf8.o: depend_stubs/unicode/regex.h
../dependencies/util/lua_utf8.o: ../dependencies/util/console.h
../dependencies/util/lua_utf8.o: ../dependencies/util/lua_util.h
../dependencies/util/lua_utf8.o: depend_stubs/limits depend_stubs/vector
../dependencies/util/lua_utf8.o: depend_stubs/lauxlib.h depend_stubs/lualib.h
../dependencies/util/lua_utf8.o: ../dependencies/util/math_util.h
../dependencies/util/lua_utf8.o: depend_stubs/cmath depend_stubs/cfloat
../dependencies/util/lua_utf8.o: ../dependencies/util/lua_wrappers_common.h
../dependencies/util/lua_utf8.o: ../dependencies/util/lua_utf8.h
../dependencies/util/lua_util.o: depend_stubs/cstdlib depend_stubs/cmath
../dependencies/util/lua_util.o: depend_stubs/string depend_stubs/map
../dependencies/util/lua_util.o: depend_stubs/sstream depend_stubs/iostream
../dependencies/util/lua_util.o: ../dependencies/util/console.h
../dependencies/util/lua_util.o: ../dependencies/util/lua_util.h
../dependencies/util/lua_util.o: depend_stubs/limits depend_stubs/vector
../dependencies/util/lua_util.o: depend_stubs/lua.h depend_stubs/lauxlib.h
../dependencies/util/lua_util.o: depend_stubs/lualib.h
../dependencies/util/lua_util.o: ../dependencies/util/math_util.h
../dependencies/util/lua_util.o: depend_stubs/cfloat
../dependencies/util/unicode_util.o: depend_stubs/cstdlib
../dependencies/util/unicode_util.o: depend_stubs/cstring depend_stubs/cstdio
../dependencies/util/unicode_util.o: ../dependencies/util/unicode_util.h
../dependencies/util/unicode_util.o: depend_stubs/string
../dependencies/util/console.o: ../dependencies/util/console.h
