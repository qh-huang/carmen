CARMEN_HOME = ../../../
include ../../Makefile.conf

MODULE_NAME = "QuickCam"
MODULE_COMMENT = "Phillips Camera Module"

SOURCES = ccvt_mmx.S ccvt_misc.c quickcam.c
TARGETS = libccvt.a libquickcam.a

all: 

libccvt.a: ccvt_mmx.o ccvt_misc.o 

libquickcam.a: quickcam.o

ccvt_mmx.o: ccvt_mmx.S
	$(CC) -c -DPACKAGE_NAME=\"\" -DPACKAGE_TARNAME=\"\" -DPACKAGE_VERSION=\"\" -DPACKAGE_STRING=\"\" -DPACKAGE_BUGREPORT=\"\" -DSTDC_HEADERS=1 -DHAVE_INTELCPU=1 -DHAVE_MMX=1  -o ccvt_mmx.o ccvt_mmx.S

ccvt_misc.o: ccvt_misc.c
	$(CC) -DPACKAGE_NAME=\"\" -DPACKAGE_TARNAME=\"\" -DPACKAGE_VERSION=\"\" -DPACKAGE_STRING=\"\" -DPACKAGE_BUGREPORT=\"\" -DSTDC_HEADERS=1 -DHAVE_INTELCPU=1 -DHAVE_MMX=1  -Wall -g -O3  -c -o ccvt_misc.o ccvt_misc.c


include ../../Makefile.rules
