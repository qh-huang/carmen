include ../Makefile.conf

#NO_GRAPHICS = 1  #dbug

MODULE_NAME = ROBOTWALKER
MODULE_COMMENT = Robotic Walker

SOURCES = robotwalker.c gnav.c gnav_interface.c valet.c valet_interface.c \
		valet_test.c

ifndef NO_GRAPHICS

SOURCES += gnav_gui.c

CFLAGS +=
IFLAGS 	+=	`$(GTK_CONFIG) --cflags`
LFLAGS 	+= 	-lmap_io -lmap_interface -lparam_interface -lgdk_imlib \
		-lglobal \
		-lglobal_graphics -lipc `$(GTK_CONFIG) --libs gthread` \
		-llocalize_interface -llaser_interface -lnavigator_interface \
		-lserial
else

CFLAGS +=
IFLAGS 	+=
LFLAGS 	+= 	-lmap_io -lmap_interface -lparam_interface -lglobal \
		-lglobal_graphics -lipc \
		-llocalize_interface -llaser_interface -lserial
endif

PUBLIC_BINARIES = 
TARGETS = robotwalker gnav valet valet_test

ifndef NO_GRAPHICS
TARGETS += gnav_gui
endif

ifdef NO_ZLIB
CFLAGS += -DNO_ZLIB
else
LFLAGS += -lz
endif


all:

robotwalker:	robotwalker.o

gnav:	gnav.o

gnav_gui:	gnav_gui.o

libgnav_interface.a:	gnav_interface.o

valet:	valet.o

libvalet_interface.a:	valet_interface.o

valet_test:	valet_test.o valet_interface.o

include $(CARMEN_HOME)/src/Makefile.rules

