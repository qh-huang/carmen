CARMEN_HOME = /home/mmde/carmen
include $(CARMEN_HOME)/src/Makefile.conf

MODULE_NAME = CANON
MODULE_COMMENT = Canon Powershot Camera Driver for CARMEN

CFLAGS += -Wno-shadow
IFLAGS += 
LFLAGS += -lparam_interface -lglobal -lipc -lusb -ljpeg

PUBLIC_INCLUDES = 
PUBLIC_LIBRARIES = 
PUBLIC_BINARIES = 
MAN_PAGES =
SOURCES = canon.c canon_server.c canon_ipc.c capture_test.c parse-log.c
TARGETS = libcanon.a libcanon_interface.a canon_server capture_test parse-log

ifndef NO_GRAPHICS		
IFLAGS += `$(GTK_CONFIG) --cflags`
LFLAGS += `$(GTK_CONFIG) --libs` -lgdk_imlib -lglobal_graphics
SOURCES += previewer.c
TARGETS += previewer
endif

# rules

all:

libcanon.a:	canon.o

libcanon_interface.a:	canon_interface.o jpegread.o

capture_test:	capture_test.o libcanon_interface.a

previewer:	previewer.o libcanon_interface.a

canon_server:	canon_server.o canon_ipc.o libcanon.a

parse-log:	parse-log.o

include $(CARMEN_HOME)/src/Makefile.rules
