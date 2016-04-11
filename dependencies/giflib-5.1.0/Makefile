# makefile for building giflib

ROOT=../..

include ../../common.mk

CC ?= gcc
CFLAGS= $(GRIT_CXX_CODEGEN)
LDFLAGS= 

GIFLIB_A= libgif.a

OBJ_FILES= dgif_lib.o \
	egif_lib.o \
	gifalloc.o \
	gif_err.o \
	gif_font.o \
	gif_hash.o \
	quantize.o \

$(GIFLIB_A): $(OBJ_FILES)
	ar -rcus $@ $(OBJ_FILES)
	ranlib $@

clean:
	rm -fv $(OBJ_FILES) $(LUA_O) 

