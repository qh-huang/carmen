CARMEN_HOME = /home/mmde/carmen
include $(CARMEN_HOME)/src/Makefile.conf

CFLAGS += -Wno-shadow
IFLAGS += 
LFLAGS += -lparam_interface -lglobal -lipc -lusb

MODULE_NAME = CANON
MODULE_COMMENT = Canon Powershot Camera Driver

SOURCES = canon.c
PUBLIC_INCLUDES = 
PUBLIC_LIBRARIES = 
PUBLIC_BINARIES = 
MAN_PAGES =

TARGETS = libcanon.a libcanon_interface.a canon_server canon_test preview_test camera_test parse-log

# rules

all:

libcanon.a:	canon.o

libcanon_interface.a:	canon_interface.o

canon_test:	canon_test.o libcanon_interface.a

preview_test:	preview_test.o libcanon_interface.a

camera_test:	camera_test.o libcanon.a

canon_server:	canon_server.o libcanon.a

interface_test:	interface_test.o libcanon_interface.a

parse-log:	parse-log.o

include $(CARMEN_HOME)/src/Makefile.rules
