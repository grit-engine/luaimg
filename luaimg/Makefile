# Makefile for building luaimg

ROOT = ..

include ../common.mk

LUA_A =  ../dependencies/lua-5.1.4/liblua.a

SOURCE = luaimg.cpp interpreter.cpp Image.cpp lua_wrappers_image.cpp
 
LUAIMG_CXXFLAGS = $(GRIT_CXX_CONFORMANCE) $(GRIT_CXX_CODEGEN) $(FREEIMAGE_CXXFLAGS) $(LUA_CXXFLAGS) -DLUA_USE_READLINE
LUAIMG_LDFLAGS  = $(FREEIMAGE_LDFLAGS) $(LUA_LDFLAGS)
LUAIMG_LDLIBS   = $(FREEIMAGE_LDLIBS) $(LUA_LDLIBS) -lm -lreadline

luaimg: $(SOURCE) $(LUA_A)
	$(CXX) $(LUAIMG_CXXFLAGS) $(SOURCE) $(LUAIMG_LDFLAGS) $(LUAIMG_LDLIBS) -o $@

depend:
	makedepend -Y -f Makefile $(SOURCE) -I depend_stubs
	sed -i 's_^[a-zA-Z_]*[.]o:_luaimg:_g' Makefile


clean:
	rm -fv luaimg

# DO NOT DELETE

luaimg: depend_stubs/cstdlib depend_stubs/string depend_stubs/vector
luaimg: depend_stubs/sstream depend_stubs/iostream depend_stubs/FreeImage.h
luaimg: depend_stubs/lua.h depend_stubs/lauxlib.h depend_stubs/lualib.h
luaimg: interpreter.h Image.h
luaimg: depend_stubs/cstdlib depend_stubs/string depend_stubs/vector
luaimg: depend_stubs/iostream depend_stubs/signal.h depend_stubs/lua.h
luaimg: depend_stubs/lauxlib.h depend_stubs/lualib.h interpreter.h
luaimg: lua_wrappers_image.h
luaimg: depend_stubs/string depend_stubs/iostream depend_stubs/FreeImage.h
luaimg: Image.h
luaimg: depend_stubs/string depend_stubs/sstream
luaimg: depend_stubs/vector depend_stubs/lua.h
luaimg: depend_stubs/lauxlib.h depend_stubs/lualib.h
luaimg: lua_wrappers_image.h Image.h
