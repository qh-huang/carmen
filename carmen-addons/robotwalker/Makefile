include ../Makefile.conf

#NO_GRAPHICS = 1  #dbug

MODULE_NAME = ROBOTWALKER
MODULE_COMMENT = Robotic Walker

SOURCES = walker_motors.c gnav.c gnav_interface.c valet.c valet_interface.c \
		valet_run.c walkerserial.c walkerserial_interface.c walker.c \
		walker_interface.c

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
TARGETS = walker_motors gnav valet valet_run walkerserial libgnav_interface.a \
		libvalet_interface.a libwalkerserial_interface.a walker \
		libwalker_interface.a

ifndef NO_GRAPHICS
TARGETS += gnav_gui
endif

ifdef NO_ZLIB
CFLAGS += -DNO_ZLIB
else
LFLAGS += -lz
endif


all:

walker:	walker.o gnav_interface.o walkerserial_interface.o

libwalker_interface.a:	walker_interface.o

walker_motors:	walker_motors.o

gnav:	gnav.o walker_interface.o

gnav_gui:	gnav_gui.o gnav_interface.o walkerserial_interface.o \
		walker_interface.o

libgnav_interface.a:	gnav_interface.o

valet:	valet.o

libvalet_interface.a:	valet_interface.o

valet_run:	valet_run.o valet_interface.o

walkerserial:	walkerserial.o walkerserial_interface.o

libwalkerserial_interface.a:	walkerserial_interface.o

include $(CARMEN_HOME)/src/Makefile.rules

