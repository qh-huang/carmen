include ../Makefile.conf

IFLAGS += -I$(JAVA_HOME)/include 
IFLAGS += -I$(JAVA_HOME)/include/linux 
IFLAGS += -I/usr/include/bsd
IFLAGS += -I ../ipc
LFLAGS += -L ../../lib
CFLAGS += -DREDHAT_52 -DREDHAT_6 -DREDHAT_71 
CFLAGS += -Wno-unused-parameter


MODULE_NAME = JAVA LIBRARIES
MODULE_COMMENT = Module for interfacing java programs with Carmen!

SOURCES = ipcjava.c
PUBLIC_LIBRARIES = libipcjava.so
TARGETS = libipcjava.so 

all:

libipcjava.so: ipcjava.o
	$(ECHO) "    ---- Creating shared library (C)"
	@ld -o $@ -G -dy $^ -L ../ipc -lipc

# rules

include ../Makefile.rules
