CC=gcc -std=gnu99 -Wall -Wshadow -Wno-unused-parameter -g #-fPIC 
AR=ar

all:	liborc.a liborc.so orctest


ORC_OBJS=orc.o rawprovider.o tcpprovider.o packet.o timespec.o clog.o
liborc.a:	$(ORC_OBJS)
	$(AR) -cr $@ $(ORC_OBJS)

liborc.so:	$(ORC_OBJS)
	$(CC) -shared -Wl,-soname,liborc.so -o $@ $(ORC_OBJS) -lc

ORCTEST_OBJS=main.o
orctest: $(ORCTEST_OBJS) orc.h orc.c
#	$(CC) $(ORCTEST_OBJS) -o $@ -lpthread -lorc -L.
	$(CC) $(ORCTEST_OBJS) -o $@ -lpthread liborc.a

%.o: %.c
	$(CC) -c $<

clean: 
	rm -f *~ a.out *.o *.a *.so
