include ../Makefile.conf

MODULE_NAME = ROBOTWALKER
MODULE_COMMENT = Robotic Walker

SOURCES = robotwalker.c gnav_map.c

ifndef NO_GRAPHICS

CFLAGS +=
IFLAGS 	+=	`$(GTK_CONFIG) --cflags`
LFLAGS 	+= 	-lmap_io -lmap_interface -lparam_interface -lgdk_imlib -lglobal \
		-lglobal_graphics -lipc `$(GTK_CONFIG) --libs gthread` \
		-llocalize_interface -llaser_interface -lserial

PUBLIC_BINARIES = 
TARGETS = robotwalker gnav_map

ifdef NO_ZLIB
CFLAGS += -DNO_ZLIB
else
LFLAGS += -lz
endif

endif


all:

robotwalker:	robotwalker.o

gnav_map:	gnav_map.o

# rules

include $(CARMEN_HOME)/src/Makefile.rules

