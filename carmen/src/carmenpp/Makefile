include ../Makefile.conf

LINK=g++

LFLAGS +=  -lreadlog -lwritelog -lmap_interface -lglobal -lipc

CFLAGS += -O3
CXXFLAGS += -O3

MODULE_NAME = CARMENPP
MODULE_COMMENT = Cyrills CPP Wrapper for Carmen

SOURCES =  *.cpp 

PUBLIC_INCLUDES  = 
PUBLIC_LIBRARIES = 
PUBLIC_BINARIES = 

TARGETS = libcarmen_carmenpp.a test test_logfile

libcarmen_carmenpp.a:  cpp_laser.o cpp_base.o cpp_simulator.o cpp_robot.o cpp_carmenmap.o cpp_logfile.o

test : test.o cpp_laser.o cpp_base.o cpp_simulator.o cpp_robot.o cpp_carmenmap.o cpp_logfile.o

test_logfile: test_logfile.o cpp_laser.o cpp_base.o cpp_simulator.o cpp_robot.o cpp_carmenmap.o cpp_logfile.o

include ../Makefile.rules
