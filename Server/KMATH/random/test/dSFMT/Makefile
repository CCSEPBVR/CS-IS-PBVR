#
#  File     : Makefile
#  Authors  : TI, NT
#  Modified : 2013/02/05 (NT)
#
#  (c) Copyright 2013 RIKEN. All rights reserved.
#

include ../../Makefile.machine


# dSFMT direct test

CFLAGS		+= -DDSFMT_MEXP=19937
DSFMTDIR	= ../../dsfmt


# uses dSFMT symbols included in c/libkm_random.a

CLIBDIR		= ../../c
CLIBFILE	= $(CLIBDIR)/libkm_random.a
CLINK		= $(LFLAGS) -lntl -lstdc++


all: test test_speed test_jump


test: makelib test.o
	$(CC) -o test test.o $(CLIBFILE) $(CLINK)

test_speed: makelib test_speed.o
	$(CXX) -o test_speed test_speed.o $(CLIBFILE) $(CLINK)

test_jump: makelib test_jump.o
	$(CXX) -o test_jump test_jump.o $(CLIBFILE) $(CLINK)


test.o: test.c
	$(CC) $(CFLAGS) -I$(DSFMTDIR) $< -o $@

test_speed.o: test_speed.cpp
	$(CXX) $(CFLAGS) -I$(DSFMTDIR) $< -o $@

test_jump.o: test_jump.cpp
	$(CXX) $(CFLAGS) -I$(DSFMTDIR) $< -o $@


makelib:
	cd $(CLIBDIR);	make

clean:
	rm -f *.o *~ test test_speed test_jump

all_clean:
	cd $(CLIBDIR);	make clean
	rm -f *.o *~ test test_speed test_jump
