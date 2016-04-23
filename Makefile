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

COMPUTING_DEPENDENCIES= echo -e '\e[0mComputing dependencies: \e[33m$<\e[0m'
COMPILING= echo -e '\e[0mCompiling: \e[32m$<\e[0m'
LINKING= echo -e '\e[0mLinking: \e[1;32m$@\e[0m'

ALL_TARGETS= \
	$(addprefix build/,$(WEAK_CPP_SRCS:%.cpp=%.weak_cpp)) \
	$(addprefix build/,$(WEAK_C_SRCS:%.c=%.weak_c)) \
	$(addprefix build/,$(CPP_SRCS)) \
	$(addprefix build/,$(C_SRCS)) \

build/%.cpp.o: %.cpp
	@$(COMPILING)
	@mkdir -p $(shell dirname $@)
	@$(CXX) -c $(CODEGEN) -std=c++11 -pedantic -Wall -Wextra $(CFLAGS) $< -o $@

build/%.c.o: %.c
	@$(COMPILING)
	@mkdir -p $(shell dirname $@)
	@$(CC) -c $(CODEGEN) -std=c99 -pedantic -Wall -Wextra $(CFLAGS) $< -o $@

build/%.weak_cpp.o: %.cpp
	@$(COMPILING)
	@mkdir -p $(shell dirname $@)
	@$(CXX) -c $(CODEGEN) $(CFLAGS) $< -o $@

build/%.weak_c.o: %.c
	@$(COMPILING)
	@mkdir -p $(shell dirname $@)
	@$(CC) -c $(CODEGEN) $(CFLAGS) $< -o $@

luaimg: $(addsuffix .o,$(ALL_TARGETS))
	@$(LINKING)
	@$(CXX) $^ $(LDFLAGS) $(LDLIBS) -o $@


# ---------
# Dev stuff
# ---------

build/%.cpp.d: %.cpp
	@$(COMPUTING_DEPENDENCIES)
	@mkdir -p $(shell dirname $@)
	@$(CXX) -MM -std=c++11 $(CFLAGS) $< -o $@ -MT $(@:%.d=%.o)

build/%.c.d: %.c
	@$(COMPUTING_DEPENDENCIES)
	@mkdir -p $(shell dirname $@)
	@$(CC) -MM -std=c99 $(CFLAGS) $< -o $@ -MT $(@:%.d=%.o)

build/%.weak_cpp.d: %.cpp
	@$(COMPUTING_DEPENDENCIES)
	@mkdir -p $(shell dirname $@)
	@$(CXX) -MM -std=c++11 $(CFLAGS) $< -o $@ -MT $(@:%.d=%.o)

build/%.weak_c.d: %.c
	@$(COMPUTING_DEPENDENCIES)
	@mkdir -p $(shell dirname $@)
	@$(CC) -MM $(CFLAGS) $< -o $@ -MT $(@:%.d=%.o)

-include makedepend.mk
ALL_DEPS = $(addsuffix .d,$(ALL_TARGETS))
depend: $(ALL_DEPS)

clean:
	rm -rfv luaimg build

-include $(ALL_DEPS)
