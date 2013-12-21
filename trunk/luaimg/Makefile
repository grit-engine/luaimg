# Makefile for building luaimg

ROOT = ..

include $(ROOT)/common.mk

LUA_A =  $(ROOT)/dependencies/lua-5.1.4/liblua.a
 
LUAIMG_CXXFLAGS = $(ICU_CXXFLAGS) $(GRIT_CXX_CONFORMANCE) $(GRIT_CXX_CODEGEN) $(FREEIMAGE_CXXFLAGS) $(LUA_CXXFLAGS) -DLUA_USE_READLINE -Wno-type-limits $(shell pkg-config freetype2 --cflags) -I $(ROOT)/dependencies/util
LUAIMG_LDFLAGS  = $(ICU_LDFLAGS) $(FREEIMAGE_LDFLAGS) $(LUA_LDFLAGS) $(shell pkg-config freetype2 --libs-only-L)
LUAIMG_LDLIBS   = $(ICU_LDLIBS) $(FREEIMAGE_LDLIBS) $(LUA_LDLIBS) -lm -lreadline $(shell pkg-config freetype2 --libs-only-l)

OBJECTS = \
	luaimg.o \
	interpreter.o \
	Image.o \
	lua_wrappers_image.o \
	text.o \
	dds.o \
	sfi.o \
	lua_utf8.o \
	lua_util.o \
	unicode_util.o \
	console.o \
	posix_sleep.o \
	io_util.o \
	colour_conversion.o

%.o: $(ROOT)/dependencies/util/%.cpp
	$(CXX) -c $(LUAIMG_CXXFLAGS) $< -o $@

%.o: %.cpp
	$(CXX) -c $(LUAIMG_CXXFLAGS) $< -o $@

luaimg.$(GRIT_EXEC_SUFFIX): $(OBJECTS) $(LUA_A) $(ICU_ARCHIVES)
	$(CXX) $^ $(LUAIMG_LDFLAGS) $(LUAIMG_LDLIBS) -o $@

depend:
	makedepend -Y -f Makefile *.cpp $(ROOT)/dependencies/util/*.cpp -I $(ROOT)/dependencies/util -I depend_stubs
	sed -i 's_^$(ROOT)/dependencies/util/\([^.]*[.]o:\)_\1_g' Makefile


PACKAGED = \
	luaimg.$(GRIT_EXEC_SUFFIX) \
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

%.tar.bz2: $(PACKAGED)
	ln -sf . $(shell basename $@ .tar.bz2)
	tar  -cjvf $@ $(patsubst %,$(shell basename $@ .tar.bz2)/%,$(PACKAGED))

%.zip: $(PACKAGED)
	zip $@ $(PACKAGED)

clean:
	rm -fv luaimg.$(GRIT_EXEC_SUFFIX) $(OBJECTS)
# DO NOT DELETE

dds.o: dds.h Image.h depend_stubs/cinttypes depend_stubs/cmath
dds.o: depend_stubs/cassert depend_stubs/ostream depend_stubs/string
dds.o: ../dependencies/util/io_util.h depend_stubs/cstdlib
dds.o: depend_stubs/cstdio depend_stubs/cstring depend_stubs/iostream
dds.o: depend_stubs/fstream ../dependencies/util/exception.h
dds.o: depend_stubs/sstream ../dependencies/util/intrinsics.h
Image.o: depend_stubs/cstdlib depend_stubs/cstdio depend_stubs/cmath
Image.o: depend_stubs/cstring depend_stubs/string depend_stubs/iostream
Image.o: depend_stubs/fstream depend_stubs/FreeImage.h
Image.o: ../dependencies/util/colour_conversion.h Image.h
Image.o: depend_stubs/cinttypes depend_stubs/cassert depend_stubs/ostream
Image.o: sfi.h
interpreter.o: depend_stubs/cstdlib depend_stubs/string depend_stubs/vector
interpreter.o: depend_stubs/iostream depend_stubs/signal.h depend_stubs/lua.h
interpreter.o: depend_stubs/lauxlib.h depend_stubs/lualib.h
interpreter.o: ../dependencies/util/lua_util.h depend_stubs/sstream
interpreter.o: depend_stubs/limits ../dependencies/util/math_util.h
interpreter.o: depend_stubs/cmath depend_stubs/cfloat
interpreter.o: ../dependencies/util/intrinsics.h
interpreter.o: ../dependencies/util/lua_utf8.h ../dependencies/util/console.h
interpreter.o: interpreter.h lua_wrappers_image.h Image.h
interpreter.o: depend_stubs/cinttypes depend_stubs/cassert
interpreter.o: depend_stubs/ostream
luaimg.o: depend_stubs/cstdlib depend_stubs/string depend_stubs/vector
luaimg.o: depend_stubs/sstream depend_stubs/iostream depend_stubs/FreeImage.h
luaimg.o: depend_stubs/lua.h depend_stubs/lauxlib.h depend_stubs/lualib.h
luaimg.o: interpreter.h Image.h depend_stubs/cinttypes depend_stubs/cmath
luaimg.o: depend_stubs/cassert depend_stubs/ostream text.h
lua_wrappers_image.o: depend_stubs/iostream depend_stubs/string
lua_wrappers_image.o: depend_stubs/sstream depend_stubs/vector
lua_wrappers_image.o: depend_stubs/limits depend_stubs/algorithm
lua_wrappers_image.o: depend_stubs/lua.h depend_stubs/lauxlib.h
lua_wrappers_image.o: depend_stubs/lualib.h ../dependencies/util/lua_util.h
lua_wrappers_image.o: ../dependencies/util/math_util.h depend_stubs/cmath
lua_wrappers_image.o: depend_stubs/cfloat ../dependencies/util/intrinsics.h
lua_wrappers_image.o: ../dependencies/util/unicode_util.h
lua_wrappers_image.o: depend_stubs/cstdlib ../dependencies/util/sleep.h
lua_wrappers_image.o: ../dependencies/util/exception.h lua_wrappers_image.h
lua_wrappers_image.o: Image.h depend_stubs/cinttypes depend_stubs/cassert
lua_wrappers_image.o: depend_stubs/ostream text.h
sfi.o: depend_stubs/cstdlib depend_stubs/cstring depend_stubs/iostream
sfi.o: depend_stubs/fstream depend_stubs/string
sfi.o: ../dependencies/util/exception.h depend_stubs/sstream
sfi.o: ../dependencies/util/intrinsics.h ../dependencies/util/io_util.h
sfi.o: depend_stubs/cstdio sfi.h Image.h depend_stubs/cinttypes
sfi.o: depend_stubs/cmath depend_stubs/cassert depend_stubs/ostream
text.o: depend_stubs/cstdlib depend_stubs/cstdio depend_stubs/cmath
text.o: depend_stubs/cstring depend_stubs/string depend_stubs/iostream
text.o: depend_stubs/fstream depend_stubs/ft2build.h
text.o: ../dependencies/util/lua_util.h depend_stubs/sstream
text.o: depend_stubs/limits depend_stubs/vector depend_stubs/lua.h
text.o: depend_stubs/lauxlib.h depend_stubs/lualib.h
text.o: ../dependencies/util/math_util.h depend_stubs/cfloat
text.o: ../dependencies/util/intrinsics.h ../dependencies/util/unicode_util.h
text.o: ../dependencies/util/exception.h text.h depend_stubs/ostream Image.h
text.o: depend_stubs/cinttypes depend_stubs/cassert
VoxelImage.o: depend_stubs/cstdlib depend_stubs/iostream
VoxelImage.o: depend_stubs/volpack.h VoxelImage.h Image.h
VoxelImage.o: depend_stubs/cinttypes depend_stubs/cmath depend_stubs/cassert
VoxelImage.o: depend_stubs/ostream depend_stubs/string
colour_conversion.o: depend_stubs/cstdlib
colour_conversion.o: depend_stubs/cmath
colour_conversion.o: depend_stubs/algorithm
colour_conversion.o: ../dependencies/util/colour_conversion.h
console.o: ../dependencies/util/console.h
io_util.o: depend_stubs/string depend_stubs/iostream
io_util.o: depend_stubs/fstream
io_util.o: ../dependencies/util/io_util.h
io_util.o: depend_stubs/cstdlib depend_stubs/cstdio
io_util.o: depend_stubs/cstring
io_util.o: ../dependencies/util/exception.h
io_util.o: depend_stubs/sstream
io_util.o: ../dependencies/util/intrinsics.h
lua_utf8.o: depend_stubs/cstdlib depend_stubs/cstdio
lua_utf8.o: depend_stubs/cassert depend_stubs/iostream
lua_utf8.o: depend_stubs/string depend_stubs/sstream
lua_utf8.o: depend_stubs/lua.h
lua_utf8.o: depend_stubs/unicode/unistr.h
lua_utf8.o: depend_stubs/unicode/uchar.h
lua_utf8.o: depend_stubs/unicode/regex.h
lua_utf8.o: ../dependencies/util/console.h
lua_utf8.o: ../dependencies/util/lua_util.h
lua_utf8.o: depend_stubs/limits depend_stubs/vector
lua_utf8.o: depend_stubs/lauxlib.h depend_stubs/lualib.h
lua_utf8.o: ../dependencies/util/math_util.h
lua_utf8.o: depend_stubs/cmath depend_stubs/cfloat
lua_utf8.o: ../dependencies/util/intrinsics.h
lua_utf8.o: ../dependencies/util/lua_wrappers_common.h
lua_utf8.o: ../dependencies/util/lua_utf8.h
lua_util.o: depend_stubs/cstdlib depend_stubs/cmath
lua_util.o: depend_stubs/string depend_stubs/map
lua_util.o: depend_stubs/sstream depend_stubs/iostream
lua_util.o: ../dependencies/util/console.h
lua_util.o: ../dependencies/util/lua_util.h
lua_util.o: depend_stubs/limits depend_stubs/vector
lua_util.o: depend_stubs/lua.h depend_stubs/lauxlib.h
lua_util.o: depend_stubs/lualib.h
lua_util.o: ../dependencies/util/math_util.h
lua_util.o: depend_stubs/cfloat
lua_util.o: ../dependencies/util/intrinsics.h
posix_sleep.o: depend_stubs/ctime depend_stubs/cassert
posix_sleep.o: depend_stubs/cstdio
unicode_util.o: depend_stubs/cstdlib
unicode_util.o: depend_stubs/cstring depend_stubs/cstdio
unicode_util.o: ../dependencies/util/unicode_util.h
unicode_util.o: depend_stubs/string
win32_sleep.o: depend_stubs/windows.h
