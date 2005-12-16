include ../Makefile.conf

LINK=g++
LFLAGS += -lparam_interface -lglobal -lipc -lm


MODULE_NAME = MAPTOOLS
MODULE_COMMENT = Tools for mapping etc.

SOURCES = linemapping.cpp

PUBLIC_INCLUDES = linemapping.h
PUBLIC_LIBRARIES = libclfconvert.a liblinemapping.a
PUBLIC_BINARIES = clf2linemap

TARGETS =  liblinemapping.a clf2linemap libmap_util.a maptool

ifndef NO_GRAPHICS
 IFLAGS += `$(GTK_CONFIG) --cflags`
 LFLAGS += -lglobal_graphics -lgdk_imlib `$(GTK_CONFIG) --libs`
endif

ifndef NO_GRAPHICS
SOURCES += map_graphics.c 
PUBLIC_INCLUDES += map_graphics.h
PUBLIC_LIBRARIES += libmap_graphics.a
TARGETS += libmap_graphics.a
endif

all:

liblinemapping.a:    linemapping.o

clf2linemap:         clf2linemap.o clfreader.o linemapping.o

libmap_util.a: map_util.o

maptool: maptool.o libmap_util.a

libmap_graphics.a:	map_graphics.o libmap_interface.a

include  ../Makefile.rules

