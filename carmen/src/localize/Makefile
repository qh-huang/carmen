include ../Makefile.conf

ifdef FOR_XAVIER
INTERFACE_LIBS = -lxavier_interface
else
INTERFACE_LIBS = -lrobot_interface
endif

CFLAGS +=
IFLAGS += 
LFLAGS += -lglobal -lparam_interface -llaser_interface -lmap_interface $(INTERFACE_LIBS) -lipc 

MODULE_NAME = LOCALIZE
MODULE_COMMENT = Markov Localization Module

SOURCES = localize.c likelihood_map.c localizecore.c localize_interface.c localize_initialize.c
PUBLIC_INCLUDES = localize_messages.h localize_interface.h localizecore.h likelihood_map.h
PUBLIC_LIBRARIES = liblocalize_interface.a liblocalize_core.a
PUBLIC_BINARIES = localize_initialize localize 
MAN_PAGES =

TARGETS = localize liblocalize_interface.a localize_initialize liblocalize_core.a

ifndef NO_ZLIB
LFLAGS += -lz
else
CFLAGS += -DNO_ZLIB
endif


# rules

all:

localize_initialize:	localize_initialize.o liblocalize_interface.a

liblocalize_interface.a:	localize_interface.o

liblocalize_core.a: localizecore.o likelihood_map.o

localize:	localize.o likelihood_map.o localizecore.o

localizegraph:	localizegraph.o likelihood_map.o liblocalize_interface.a

include ../Makefile.rules



