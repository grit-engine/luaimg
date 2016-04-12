# ============================
# Makefile for building luaimg
# ============================


# -----------------------
# User-overrideable parts
# -----------------------

CXX?= g++ 
CC?= gcc
OPT?=-O3 -DNDEBUG
ARCH?= -march=native -mtune=native


# -----------------
# Compilation input
# -----------------

include dependencies/giflib-5.1.0/grit.mk
include dependencies/grit-lua/grit.mk
include dependencies/grit-util/grit.mk
include dependencies/squish-1.11/grit.mk

ICU_LDLIBS= \
	/usr/lib/x86_64-linux-gnu/libicui18n.a \
	/usr/lib/x86_64-linux-gnu/libicuuc.a \
	/usr/lib/x86_64-linux-gnu/libicudata.a \
	-ldl \


FREEIMAGE_LDFLAGS= \
	-lfreeimage \


WEAK_C_SRCS= \
	$(addprefix dependencies/giflib-5.1.0/,$(GIFLIB_WEAK_C_SRCS)) \
	$(addprefix dependencies/grit-lua/,$(LUA_WEAK_C_SRCS)) \
	$(addprefix dependencies/grit-util/,$(UTIL_WEAK_C_SRCS)) \
	$(addprefix dependencies/squish-1.11/,$(SQUISH_WEAK_C_SRCS)) \
	$(FREEIMAGE_WEAK_C_SRCS) \
	$(ICU_WEAK_C_SRCS) \

WEAK_CPP_SRCS= \
	$(addprefix dependencies/giflib-5.1.0/,$(GIFLIB_WEAK_CPP_SRCS)) \
	$(addprefix dependencies/grit-lua/,$(LUA_WEAK_CPP_SRCS)) \
	$(addprefix dependencies/grit-util/,$(UTIL_WEAK_CPP_SRCS)) \
	$(addprefix dependencies/squish-1.11/,$(SQUISH_WEAK_CPP_SRCS)) \
	$(FREEIMAGE_WEAK_CPP_SRCS) \
	$(ICU_WEAK_CPP_SRCS) \

C_SRCS= \
	$(addprefix dependencies/giflib-5.1.0/,$(GIFLIB_C_SRCS)) \
	$(addprefix dependencies/grit-lua/,$(LUA_C_SRCS)) \
	$(addprefix dependencies/grit-util/,$(UTIL_C_SRCS)) \
	$(addprefix dependencies/squish-1.11/,$(SQUISH_C_SRCS)) \
	$(FREEIMAGE_C_SRCS) \
	$(ICU_C_SRCS) \

CPP_SRCS= \
	$(addprefix dependencies/giflib-5.1.0/,$(GIFLIB_CPP_SRCS)) \
	$(addprefix dependencies/grit-lua/,$(LUA_CPP_SRCS)) \
	$(addprefix dependencies/grit-util/,$(UTIL_CPP_SRCS)) \
	$(addprefix dependencies/squish-1.11/,$(SQUISH_CPP_SRCS)) \
	$(FREEIMAGE_CPP_SRCS) \
	$(ICU_CPP_SRCS) \
	dds.cpp \
	gif.cpp \
	image.cpp \
	interpreter.cpp \
	luaimg.cpp \
	lua_wrappers_image.cpp \
	sfi.cpp \
	text.cpp \

INCLUDE_DIRS= \
	$(addprefix dependencies/giflib-5.1.0/,$(GIFLIB_INCLUDE_DIRS)) \
	$(addprefix dependencies/grit-lua/,$(LUA_INCLUDE_DIRS)) \
	$(addprefix dependencies/grit-util/,$(UTIL_INCLUDE_DIRS)) \
	$(addprefix dependencies/squish-1.11/,$(SQUISH_INCLUDE_DIRS)) \
	$(FREEIMAGE_INCLUDE_DIRS) \
	$(ICU_INCLUDE_DIRS) \

CFLAGS= \
	$(GIFLIB_DEFS:%=-D%) \
	$(LUA_DEFS:%=-D%) \
	$(UTIL_DEFS:%=-D%) \
	$(SQUISH_DEFS:%=-D%) \
	$(FREEIMAGE_DEFS:%=-D%) \
	$(ICU_DEFS:%=-D%) \
	$(INCLUDE_DIRS:%=-I%)  \
	$(shell pkg-config freetype2 --cflags) \

LDFLAGS= \
	$(GIFLIB_LDFLAGS) \
	$(LUA_LDFLAGS) \
	$(UTIL_LDFLAGS) \
	$(SQUISH_LDFLAGS) \
	$(FREEIMAGE_LDFLAGS) \
	$(ICU_LDFLAGS) \
	$(shell pkg-config freetype2 --libs-only-L) \

LDLIBS= \
	$(GIFLIB_LDLIBS) \
	$(LUA_LDLIBS) \
	$(UTIL_LDLIBS) \
	$(SQUISH_LDLIBS) \
	$(FREEIMAGE_LDLIBS) \
	$(ICU_LDLIBS) \
	$(shell pkg-config freetype2 --libs-only-l) \
	-lreadline \
	-lm \

CODEGEN= \
	$(OPT) \
    $(ARCH) \
	-Wno-type-limits \
	-Wno-deprecated \
	-g \
	-ffast-math \


# -----------
# Build rules
# -----------

COMPILING= echo "Compiling: [32m$<[0m"
LINKING= echo "Linking: [1;32m$@[0m"


build/%.cpp_o: %.cpp
	@$(COMPILING)
	@mkdir -p $(shell dirname $@)
	@$(CXX) -c $(CODEGEN) -std=c++11 -pedantic -Wall -Wextra $(CFLAGS) $< -o $@

build/%.c_o: %.c
	@$(COMPILING)
	@mkdir -p $(shell dirname $@)
	@$(CC) -c $(CODEGEN) -std=c99 -pedantic -Wall -Wextra $(CFLAGS) $< -o $@

build/%.weak_cpp_o: %.cpp
	@$(COMPILING)
	@mkdir -p $(shell dirname $@)
	@$(CXX) -c $(CODEGEN) $(CFLAGS) $< -o $@

build/%.weak_c_o: %.c
	@$(COMPILING)
	@mkdir -p $(shell dirname $@)
	@$(CC) -c $(CODEGEN) $(CFLAGS) $< -o $@

luaimg: $(addprefix build/,$(WEAK_CPP_SRCS:%.cpp=%.weak_cpp_o)) $(addprefix build/,$(WEAK_C_SRCS:%.c=%.weak_c_o)) $(addprefix build/,$(CPP_SRCS:%.cpp=%.cpp_o)) $(addprefix build/,$(C_SRCS:%.c=%.c_o))
	@$(LINKING)
	@$(CXX) $^ $(LDFLAGS) $(LDLIBS) -o $@


# ---------
# Dev stuff
# ---------

depend:
	makedepend -f- $(CFLAGS) $(WEAK_CPP_SRCS) | sed 's|\([^.]*\)[.]o:|build/\1.weak_cpp_o:|g' > makedepend.mk
	makedepend -f- $(CFLAGS) $(WEAK_C_SRCS) | sed 's|\([^.]*\)[.]o:|build/\1.weak_c_o:|g' >> makedepend.mk
	makedepend -f- $(CFLAGS) $(CPP_SRCS) | sed 's|\([^.]*\)[.]o:|build/\1.cpp_o:|g' >> makedepend.mk
	makedepend -f- $(CFLAGS) $(C_SRCS) | sed 's|\([^.]*\)[.]o:|build/\1.c_o:|g' >> makedepend.mk


clean:
	rm -rfv luaimg build

-include makedepend.mk
