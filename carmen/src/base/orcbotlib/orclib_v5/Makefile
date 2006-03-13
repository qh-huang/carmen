CC = gcc
CFLAGS = -std=gnu99 -Wall -Wshadow -Wno-unused-parameter -g -fPIC -I ~/Orcboard/orc/orc-firmware/common-mcu

orc_test: orc_test.c orc.h orc.c clog.h clog.c log.h  orcconstants.h packet.h packet.c rawprovider.c tcpprovider.c timespec.h timespec.c
	$(CC) $(CFLAGS) -o orc_test orc_test.c orc.c clog.c packet.c rawprovider.c tcpprovider.c timespec.c -lpthread 

orc_calib: orc_calib.c orc.h orc.c clog.h clog.c log.h  orcconstants.h packet.h packet.c rawprovider.c tcpprovider.c timespec.h timespec.c
	$(CC) $(CFLAGS) -o orc_calib orc_calib.c orc.c clog.c packet.c rawprovider.c tcpprovider.c timespec.c -lpthread 

orc_calib2: orc_calib2.c orc.h orc.c clog.h clog.c log.h  orcconstants.h packet.h packet.c rawprovider.c tcpprovider.c timespec.h timespec.c
	$(CC) $(CFLAGS) -o orc_calib2 orc_calib2.c orc.c clog.c packet.c rawprovider.c tcpprovider.c timespec.c -lpthread 

clean:
	rm -f *.o *~ liborc.a liborc.so

