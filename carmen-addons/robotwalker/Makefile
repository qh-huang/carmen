include ../Makefile.conf

#NO_GRAPHICS = 1  #dbug

MODULE_NAME = ROBOTWALKER
MODULE_COMMENT = Robotic Walker

SOURCES = walker_motors.c valet.c valet_interface.c valet_run.c \
		walkerserial.c walkerserial_interface.c walker.c \
		walker_interface.c

CFLAGS +=
IFLAGS 	+=
LFLAGS 	+= 	-lmap_io -lmap_interface -lparam_interface -lglobal \
		-lipc -lroomnav_interface -lnavigator_interface \
		-llocalize_interface -llaser_interface -lserial \
		-lrobot_interface

ifndef NO_GRAPHICS

SOURCES += walker_gui.c

CFLAGS +=
IFLAGS 	+=	`$(GTK_CONFIG) --cflags`
LFLAGS 	+= 	-lgdk_imlib -lglobal_graphics `$(GTK_CONFIG) --libs gthread`

endif

PUBLIC_BINARIES = 
TARGETS = walker_motors valet valet_run walkerserial \
		libvalet_interface.a libwalkerserial_interface.a walker \
		libwalker_interface.a

ifndef NO_GRAPHICS
TARGETS += walker_gui
endif

ifdef NO_ZLIB
CFLAGS += -DNO_ZLIB
else
LFLAGS += -lz
endif


all:

walker:	walker.o walkerserial_interface.o

libwalker_interface.a:	walker_interface.o

walker_motors:	walker_motors.o

walker_gui:	walker_gui.o walkerserial_interface.o walker_interface.o

valet:	valet.o

libvalet_interface.a:	valet_interface.o

valet_run:	valet_run.o valet_interface.o

walkerserial:	walkerserial.o walkerserial_interface.o

libwalkerserial_interface.a:	walkerserial_interface.o

include $(CARMEN_HOME)/src/Makefile.rules

