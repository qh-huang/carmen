include ../Makefile.conf

NO_GRAPHICS = 1  #dbug

MODULE_NAME = ROOMNAV
MODULE_COMMENT = Room-Based Navigator

SOURCES = roomnav.c roomnav_interface.c

ifndef NO_GRAPHICS

CFLAGS +=
IFLAGS 	+=	`$(GTK_CONFIG) --cflags`
LFLAGS 	+= 	-lmap_io -lmap_interface -lparam_interface -lgdk_imlib \
		-lglobal -lglobal_graphics \
		-lipc `$(GTK_CONFIG) --libs gthread` \
		-llocalize_interface -llaser_interface -lnavigator_interface \
		-lrobot_interface
else

CFLAGS +=	-DNO_GRAPHICS
IFLAGS 	+=
LFLAGS 	+= 	-lmap_io -lmap_interface -lparam_interface -lglobal \
		-lglobal_graphics -lipc -llocalize_interface
endif

PUBLIC_INCLUDES = roomnav.h roomnav_messages.h roomnav_interface.h
PUBLIC_LIBRARIES = libroomnav_interface.a
PUBLIC_BINARIES = roomnav

TARGETS = roomnav libroomnav_interface.a

ifdef NO_ZLIB
CFLAGS += -DNO_ZLIB
else
LFLAGS += -lz
endif


all:

roomnav:	roomnav.o

libroomnav_interface.a:		roomnav_interface.o

include $(CARMEN_HOME)/src/Makefile.rules
