include ../Makefile.conf

#NO_GRAPHICS = 1  #dbug

MODULE_NAME = ROBOTWALKER
MODULE_COMMENT = Robotic Walker

SOURCES = robotwalker.c gnav.c gnav_interface.c valet.c valet_interface.c

ifndef NO_GRAPHICS

CFLAGS +=
IFLAGS 	+=	`$(GTK_CONFIG) --cflags`
LFLAGS 	+= 	-lmap_io -lmap_interface -lparam_interface -lgdk_imlib -lglobal \
		-lglobal_graphics -lipc `$(GTK_CONFIG) --libs gthread` \
		-llocalize_interface -llaser_interface -lserial
else

CFLAGS +=
IFLAGS 	+=
LFLAGS 	+= 	-lmap_io -lmap_interface -lparam_interface -lglobal \
		-lglobal_graphics -lipc \
		-llocalize_interface -llaser_interface -lserial
endif

PUBLIC_BINARIES = 
TARGETS = robotwalker gnav autovalet

ifdef NO_ZLIB
CFLAGS += -DNO_ZLIB
else
LFLAGS += -lz
endif


all:

robotwalker:	robotwalker.o

gnav:	gnav.o

libgnav_interface.a:	gnav_interface.o

valet:	valet.o

libvalet_interface.a:	valet_interface.o


include $(CARMEN_HOME)/src/Makefile.rules

