include ../Makefile.conf


MODULE_NAME = DOT
MODULE_COMMENT = Dynamic Object Tracker

SOURCES = dot.c dot_interface.c

CFLAGS += -pg
IFLAGS 	+=
LFLAGS 	+= 	-lparam_interface -lglobal -llocalize_interface -lgeometry \
		-lipc -llaser_interface -llocalize_core \
		-lrobot_interface -lmap_interface -lgsl -lgslcblas

ifndef NO_ZLIB
LFLAGS += -lz
endif

PUBLIC_INCLUDES = dot_interface.h dot_messages.h dot.h
PUBLIC_LIBRARIES = libdot_interface.a
PUBLIC_BINARIES = dot

TARGETS = dot libdot_interface.a

ifndef NO_GRAPHICS
IFLAGS 	+= `$(GTK_CONFIG) --cflags`
LFLAGS 	+= -lglobal_graphics -lgdk_imlib `$(GTK_CONFIG) --libs`
SOURCES += cluster.c
TARGETS += cluster
endif

all:

dot: dot.o

libdot_interface.a: dot_interface.o

ifndef NO_GRAPHICS
cluster: cluster.o
endif

include $(CARMEN_HOME)/src/Makefile.rules
