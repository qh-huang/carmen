include ../Makefile.conf

LFLAGS += -lglobal -lipc

ifndef NO_READLINE
LFLAGS += -lreadline 
ifndef FOUND_SUSE 
LFLAGS += -ltermcap
endif
ifeq ($(READLINE),PRE_42)
CFLAGS += -DUSE_READLINE -DREADLINE_PRE42
else
CFLAGS += -DUSE_READLINE
endif
endif

ifndef NO_TCPD
LFLAGS += -lwrap -lnsl
else
CFLAGS += -DNO_TCPD
endif

MODULE_NAME = "ANT"
MODULE_COMMENT = "(ANT's not TCX) - A simple communications package"

SOURCES = sockutils.c ant.c testServer.c testClient.c

PUBLIC_BINARIES = 
PUBLIC_INCLUDES = ant.h sockutils.h
PUBLIC_LIBRARIES = libant_lite.a libant_lite_noipc.a libant.a
MAN_PAGES =

TARGETS = libant.a libant_lite.a libant_lite_noipc.a testServer testClient 

all:

libant.a: ant.o sockutils.o 

libant_lite.a:	ant_lite.c sockutils.o
	$(ECHO) "    ---- Compiling ant_lite.c to ant_lite.o (C)"
	$(SILENT) $(CC) $(CFLAGS) -DUSE_IPC $(IFLAGS) -c ant_lite.c -o ant_lite.o
	$(ECHO) "    ---- Archiving ant_lite.o sockutils.o into $@ (C)"
	$(SILENT) $(AR) $@ ant_lite.o sockutils.o
	$(SILENT) $(RANLIB) $@

libant_lite_noipc.a:	ant_lite.c sockutils.o
	$(ECHO) "    ---- Compiling ant_lite.c to ant_lite.o (C)"
	$(SILENT) $(CC) $(CFLAGS) $(IFLAGS) -c ant_lite.c -o ant_lite.o
	$(ECHO) "    ---- Archiving ant_lite.o sockutils.o into $@ (C)"
	$(SILENT) $(AR) $@ ant_lite.o sockutils.o
	$(SILENT) $(RANLIB) $@

testServer:	testServer.o libant_lite.a
#	$(ECHO) "    ---- Linking $^ to $@ (C)"
#	$(SILENT) $(CC) $(CFLAGS) $(IFLAGS) -o $@ $< -L. -lant_lite_noipc $(LFLAGS)

testClient:	testClient.o libant_lite.a
#	$(ECHO) "    ---- Linking $^ to $@ (C)"
#	$(SILENT) $(CC) $(CFLAGS) $(IFLAGS) -o $@ $< -L. -lant_lite $(LFLAGS)

test_readline: test_readline.o

include ../Makefile.rules
