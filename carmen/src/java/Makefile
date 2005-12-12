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
TARGETS = libipcjava.so Carmen.jar TestBase.class TestRobot.class

CLASSFILES = Carmen/*.class IPC/*.class

libipcjava.so: ipcjava.o
	$(ECHO) "    ---- Creating shared library (C)"
	@ld -o $@ -G -dy $^ -L ../ipc -lipc

Carmen.jar: 
	$(ECHO) "    ---- Compiling IPC (Java)"
	$(SILENT) javac -d . IPC/*.java
	$(ECHO) "    ---- Compiling Carmen (Java)"
	$(SILENT) javac -classpath . -d . Carmen/*.java
	$(ECHO) "    ---- Creating $@"
	$(SILENT) jar cf Carmen.jar Carmen/*.class IPC/*.class 

%.class: %.java
	$(ECHO) "    ---- Compiling $@ (Java)"
	$(SILENT) javac -classpath Carmen.jar:. -d . $<

# rules

include ../Makefile.rules


