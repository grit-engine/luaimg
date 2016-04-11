# (c) David Cunningham 2011, Licensed under the MIT license: http://www.opensource.org/licenses/mit-license.php

# This Makefile is used to avoid duplication between the other makefiles in this repository.
# If you want to override its variables, you should 

# Other makefiles:
## grit_core/linux/Makefile
## gtasa/linux/Makefile
## dependencies/grit_ogre/Makefile (in mercurial)
## dependencies/lua-5.2.3/Makefile
## 

-include $(ROOT)/user.mk


################################################################################
#THINGS THAT CAN BE OVERIDDEN IN user.mk
################################################################################

#override to substitute your own compiler
CXX ?= g++ 
CC ?= gcc

#override to change general optimisation flags
GRIT_OPTIMISE          ?= -DNDEBUG -O3
#override to dbg to use unoptimised build of ogre
GRIT_OGRE_OPT          ?= opt
#set to true if you want profile support
GRIT_GOOGLE_PERF_TOOLS ?= false
#override to build for a different architecture
GRIT_ARCH              ?= -march=native -mtune=native
#override to distinguish executables
GRIT_EXEC_SUFFIX       ?= linux.$(shell uname -m)


#override for fine-grained control of the top-level (i.e. non-dependency related) compiler args
GRIT_C_CONFORMANCE ?= -std=c99 -Wall -Wextra -Wno-deprecated
GRIT_CXX_CONFORMANCE ?= -std=c++0x -Wall -Wextra -Wno-deprecated
GRIT_CXX_CODEGEN ?= -g -ffast-math $(GRIT_ARCH) $(GRIT_OPTIMISE) 
GRIT_BASE_CXXFLAGS ?= $(GRIT_CXX_CONFORMANCE) $(GRIT_CXX_CODEGEN) -I $(ROOT)/dependencies/util
GRIT_BASE_LDFLAGS  ?=
GRIT_BASE_LDLIBS   ?= -lrt


COMPILING ?= echo "Compiling: [32m$<[0m"
LINKING   ?= echo "Linking: [1;32m$@[0m"


################################################################################
# FREEIMAGE
################################################################################

FREEIMAGE_CXXFLAGS ?= 

FREEIMAGE_LDFLAGS  ?=

FREEIMAGE_LDLIBS   ?= -lfreeimage


################################################################################
# LUA
################################################################################

LUA_CONF_FLAGS ?= -DLUA_USE_APICHECK -DLUA_USE_MKSTEMP

LUA_ARCHIVES   ?= $(ROOT)/dependencies/grit-lua/liblua.a

LUA_CXXFLAGS   ?= -isystem $(ROOT)/dependencies/grit-lua $(LUA_CONF_FLAGS)

LUA_LDFLAGS    ?=

LUA_LDLIBS     ?= $(LUA_ARCHIVES) 


################################################################################
# SQUISH
################################################################################

SQUISH_ARCHIVES   ?= $(ROOT)/dependencies/squish-1.11/libsquish.a

SQUISH_CXXFLAGS   ?= -isystem $(ROOT)/dependencies/squish-1.11

SQUISH_LDFLAGS    ?=

SQUISH_LDLIBS     ?= $(SQUISH_ARCHIVES) 


################################################################################
# GIFLIB
################################################################################

GIFLIB_ARCHIVES   ?= $(ROOT)/dependencies/giflib-5.1.0/libgif.a

GIFLIB_CXXFLAGS   ?= -isystem $(ROOT)/dependencies/giflib-5.1.0

GIFLIB_LDFLAGS    ?=

GIFLIB_LDLIBS     ?= $(GIFLIB_ARCHIVES) 


################################################################################
# ICU
################################################################################

ICU_ARCHIVES ?= /usr/lib/x86_64-linux-gnu/libicui18n.a \
		/usr/lib/x86_64-linux-gnu/libicuuc.a \
		/usr/lib/x86_64-linux-gnu/libicudata.a \

ICU_CXXFLAGS ?=

ICU_LDFLAGS  ?= -ldl

ICU_LDLIBS   ?= $(ICU_ARCHIVES)


################################################################################
# GOOGLE PERFORMANCE TOOLS
################################################################################

PERF_CXXFLAGS ?= -DUSE_GOOGLE_PERF_TOOLS

PERF_LDLIBS   ?= /usr/lib/libprofiler.a

PERF_LDFLAGS  ?=


# vim: shiftwidth=8:tabstop=8:noexpandtab
