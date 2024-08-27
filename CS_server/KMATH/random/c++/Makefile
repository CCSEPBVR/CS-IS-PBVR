#
#
#  File     : Makefile
#  Authors  : TI, NT
#  Modified : 2013/02/04 (NT)
#
#  (c) Copyright 2013 RIKEN. All rights reserved.
#
#

include ../Makefile.machine


DSFMTDIR = ../dsfmt


LIB	= libkm_random.a

OBJ	= \
	$(DSFMTDIR)/dSFMT.o         \
	$(DSFMTDIR)/dSFMT-jump.o    \
	$(DSFMTDIR)/km_dsfmt_poly.o \
	$(DSFMTDIR)/km_dsfmt_jump.o \
	kmath_random.o

$(LIB): $(OBJ)
	rm -f $(LIB)
	ar $(ARFLAGS) cr $(LIB) $(OBJ)

.SUFFIXES: .c .o
.c.o:
	$(CC) $(CFLAGS) $< -o $@

.SUFFIXES: .cpp .o
.cpp.o:
	$(CXX) $(CPPFLAGS) $< -o $@

clean:
	rm -f $(DSFMTDIR)/*.o $(DSFMTDIR)/*~ *.o *~

all_clean:
	rm -f $(DSFMTDIR)/*.o $(DSFMTDIR)/*~ *.o *~ $(LIB)
