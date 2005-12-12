include ../Makefile.conf

LFLAGS += -lipc

ifndef NO_GRAPHICS
IFLAGS += `$(GTK_CONFIG) --cflags`
LFLAGS += `$(GTK_CONFIG) --libs`
else
CFLAGS += -DNO_GRAPHICS
endif

MODULE_NAME = "GLOBAL"
MODULE_COMMENT = "CARMEN global functions"

SOURCES = global.c carmen_stdio.c geometry.c pswrap.c serial.c global_test.c \
	  carmen-config.c keyctrl.c
PUBLIC_INCLUDES = global.h carmen_stdio.h ipc_wrapper.h geometry.h pswrap.h \
	  	  carmen.h serial.h keyctrl.h
PUBLIC_LIBRARIES = libglobal.a libgeometry.a libpswrap.a libserial.a \
		   libkeyctrl.a
PUBLIC_BINARIES = carmen-config
TARGETS = libglobal.a libgeometry.a libpswrap.a libserial.a global_test \
	  carmen-config libkeyctrl.a

CHECK_CONFIG = $(shell if [ -f carmen-config.c ]; then echo "1"; fi;)

ifneq ($(CHECK_CONFIG),1)
error:
	$(ECHO) "I can't find carmen-configure.c. Did you remember to run\
(or re-run) configure?"
endif

ifndef NO_GRAPHICS
SOURCES += global_graphics.c
PUBLIC_INCLUDES += global_graphics.h carmen_graphics.h 
PUBLIC_LIBRARIES += libglobal_graphics.a 
TARGETS += libglobal_graphics.a
endif

ifndef NO_LIBJPEG
SOURCES += global_jpeg.c
CFLAGS += -DHAVE_JPEG
endif

ifndef NO_LIBART
SOURCES += artwrap.c
PUBLIC_INCLUDES += artwrap.h
PUBLIC_LIBRARIES += libartwrap.a
TARGETS += libartwrap.a
CFLAGS += -DHAVE_LIBART
endif

libglobal.a:		global.o ipc_wrapper.o carmen_stdio.o

ifndef NO_LIBJPEG
libglobal_graphics.a:	global_graphics.o global_jpeg.o
else
libglobal_graphics.a:	global_graphics.o
endif

libgeometry.a:		geometry.o

libpswrap.a:		pswrap.o

libartwrap.a:		artwrap.o

libserial.a:		serial.o

libkeyctrl.a:   	keyctrl.o

global_test: 		global_test.o libglobal.a

carmen-config: 		carmen-config.o

include ../Makefile.rules
