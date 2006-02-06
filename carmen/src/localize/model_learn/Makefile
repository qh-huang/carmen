include ../../Makefile.conf

SUBDIRS = learn_model

CC = $(CXX)
LINK = $(CXX)

CFLAGS += 
IFLAGS += 
LFLAGS += -lreadlog -lglobal -lparam_interface -lipc 

MODULE_NAME = MODEL-LEARN
MODULE_COMMENT = A program for learning motion models.

SOURCES = mt-rand.c basic.c map.c lowMap.c low.c learn.c slam.cpp 
PUBLIC_INCLUDES = l
PUBLIC_LIBRARIES = 
PUBLIC_BINARIES = model_learner
MAN_PAGES =

TARGETS = model_learner

# rules

model_learner: mt-rand.o basic.o map.o lowMap.o low.o learn.o \
	slam.o 

include ../../Makefile.rules



