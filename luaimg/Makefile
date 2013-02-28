# makefile for building Lua

ROOT=..

include ../common.mk

LUA_A= ../dependencies/lua-5.1.4/liblua.a

luaimg: luaimg.cpp interpreter.cpp Image.cpp $(LUA_A)
	$(CXX) $(GRIT_CXX_CONFORMANCE) $(GRIT_CXX_CODEGEN) $(LUA_CXXFLAGS) $(FREEIMAGE_CXXFLAGS) $^ $(LUA_A) $(FREEIMAGE_LDFLAGS) $(FREEIMAGE_LDLIBS) -lm -o $@

clean:
	rm -fv luaimg
