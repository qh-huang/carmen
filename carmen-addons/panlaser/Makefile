include ../Makefile.conf

MODULE_NAME = "PANLASER"
MODULE_COMMENT = "Adds pose information to range readings from a panning laser (by B. Gerkey)."

CFLAGS +=
IFLAGS += 
# NOTE: we should really be using g++ to link here, but I don't see a way
#       to do that using the rules found in Makefile.rules.  Explicitly
#       linking to stdc++ isn't great, but it works.
LFLAGS += -lparam_interface -lipc -lglobal -L../segway -lsegway_interface -llaser_interface -L../amtec -lamtec_interface -lstdc++ -L. -lgzvector
SOURCES = panlaser.cc panlaser_interface.c panlaser_main.cc GzVector.cc
PUBLIC_INCLUDES = panlaser_interface.h panlaser_messages.h
PUBLIC_LIBRARIES = libpanlaser_interface.a
PUBLIC_BINARIES = panlaser
TARGETS = panlaser libpanlaser_interface.a libgzvector.a

all:

panlaser:			panlaser.o panlaser_main.o

libpanlaser_interface.a:	panlaser_interface.o

libgzvector.a:			GzVector.o

# rules

include $(CARMEN_HOME)/src/Makefile.rules
