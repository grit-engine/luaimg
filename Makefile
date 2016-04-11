# Makefile for building luaimg

ROOT = .

include $(ROOT)/common.mk

LUAIMG_CXXFLAGS = $(ICU_CXXFLAGS) $(GRIT_CXX_CONFORMANCE) $(GRIT_CXX_CODEGEN) $(FREEIMAGE_CXXFLAGS) $(LUA_CXXFLAGS) $(SQUISH_CXXFLAGS) $(GIFLIB_CXXFLAGS) -DLUA_USE_READLINE -Wno-type-limits $(shell pkg-config freetype2 --cflags) -I $(ROOT)/dependencies/grit-util
LUAIMG_LDFLAGS  = $(ICU_LDFLAGS) $(FREEIMAGE_LDFLAGS) $(LUA_LDFLAGS) $(shell pkg-config freetype2 --libs-only-L)
LUAIMG_LDLIBS   = $(ICU_LDLIBS) $(FREEIMAGE_LDLIBS) $(LUA_LDLIBS) -lm -lreadline $(shell pkg-config freetype2 --libs-only-l)

OBJECTS = \
	colour_conversion.o \
	console.o \
	dds.o \
	gif.o \
	image.o \
	interpreter.o \
	io_util.o \
	luaimg.o \
	lua_stack.o \
	lua_utf8.o \
	lua_util.o \
	lua_wrappers_image.o \
	posix_sleep.o \
	sfi.o \
	text.o \
	unicode_util.o \

%.o: $(ROOT)/dependencies/grit-util/%.cpp
	$(CXX) -c $(LUAIMG_CXXFLAGS) $< -o $@

%.o: %.cpp
	$(CXX) -c $(LUAIMG_CXXFLAGS) $< -o $@

luaimg.$(GRIT_EXEC_SUFFIX): $(OBJECTS) $(LUA_ARCHIVES) $(ICU_ARCHIVES) $(SQUISH_ARCHIVES) $(GIFLIB_ARCHIVES)
	$(CXX) $^ $(LUAIMG_LDFLAGS) $(LUAIMG_LDLIBS) $(SQUISH_LDFLAGS) $(SQUISH_LDLIBS) $(GIFLIB_LDFLAGS) $(GIFLIB_LDLIBS) -o $@

depend:
	makedepend -f- *.cpp $(ROOT)/dependencies/grit-util/*.cpp -I $(ROOT)/dependencies/grit-util > makedepend.mk
	sed -i 's_^$(ROOT)/dependencies/grit-util/\([^.]*[.]o:\)_\1_g' makedepend.mk


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
    examples/alpha_gen.lua \
    examples/bresenham_pattern.lua \
    examples/colour_map.lua \
    examples/colour_map_scurve.lua \
    examples/compass_rose.lua \
    examples/compass_ticks.lua \
    examples/cubemap.lua \
    examples/dds_decompose.lua \
    examples/font.lua \
    examples/gui_slider_tex.lua \
    examples/lena_blueprint.lua \
    examples/lena_std.png \
    examples/logo.lua \
    examples/mandelbrot.lua \
    examples/meme.lua \
    examples/money.lua \
    examples/money_input.png \
    examples/northern_lights.lua \
    examples/selftest.lua \
    examples/unblend.lua \
    examples/volumemap.lua \

%.tar.bz2: $(PACKAGED)
	ln -sf . $(shell basename $@ .tar.bz2)
	tar  -cjvf $@ $(patsubst %,$(shell basename $@ .tar.bz2)/%,$(PACKAGED))

%.zip: $(PACKAGED)
	zip $@ $(PACKAGED)

clean:
	rm -fv luaimg.$(GRIT_EXEC_SUFFIX) $(OBJECTS)

-include makedepend.mk

