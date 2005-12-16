include ../Makefile.conf

LINK=g++
LFLAGS += -lparam_interface -lglobal -lipc -lm


MODULE_NAME = MAPTOOLS
MODULE_COMMENT = Tools for mapping etc.

SOURCES = linemapping.cpp

PUBLIC_INCLUDES = linemapping.h
PUBLIC_LIBRARIES = libclfconvert.a liblinemapping.a
PUBLIC_BINARIES = clf2linemap

TARGETS =  liblinemapping.a \
	clf2linemap

all:

liblinemapping.a:    linemapping.o

clf2linemap:         clf2linemap.o clfreader.o linemapping.o

include  ../Makefile.rules

