CARMEN_HOME = /home/mmde/carmen
include $(CARMEN_HOME)/src/Makefile.conf

CFLAGS += -Wno-shadow
IFLAGS += 
LFLAGS += -lparam_interface -lglobal -lipc -lusb

MODULE_NAME = CANON
MODULE_COMMENT = Canon Powershot Camera Driver for CARMEN

SOURCES = canon.c canon_server.c capture_test.c preview_test.c parse-log.c
PUBLIC_INCLUDES = 
PUBLIC_LIBRARIES = 
PUBLIC_BINARIES = 
MAN_PAGES =

TARGETS = libcanon.a libcanon_interface.a canon_server capture_test preview_test parse-log

# rules

all:

libcanon.a:	canon.o

libcanon_interface.a:	canon_interface.o

capture_test:	capture_test.o libcanon_interface.a

preview_test:	preview_test.o libcanon_interface.a

canon_server:	canon_server.o libcanon.a

parse-log:	parse-log.o

include $(CARMEN_HOME)/src/Makefile.rules
