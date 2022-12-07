#
#  File     : Makefile
#  Authors  : TI, NT
#  Modified : 2013/02/05 (NT)
#
#  (c) Copyright 2013 RIKEN. All rights reserved.
#

include ../Makefile.machine

CLIBDIR		= ../c
CLIBFILE	= $(CLIBDIR)/libkm_random.a

CPPLIBDIR	= ../c++
CPPLIBFILE	= $(CPPLIBDIR)/libkm_random.a

F90LIBDIR 	= ../f90
F90LIBFILE	= $(F90LIBDIR)/libkm_random.a

CLINK		= $(LFLAGS) -lntl -lstdc++
CPPLINK		= $(LFLAGS) -lntl
F90LINK		= $(LFLAGS) -lntl -lstdc++

CFLAGS		= -fopenmp -L${CLIBDIR}
CPPFLAGS	= -fopenmp -L${CPPLIBDIR}
F90FLAGS	= -fopenmp -L${F90LIBDIR}

all: 	test_c test_cpp test_cpp2 test_f90 test_c_speed


test_c: makelib test_c.o
	$(CC) -o test_c test_c.o $(CLIBFILE) $(CLINK)

test_cpp: makelib test_cpp.o
	$(CXX) -o test_cpp test_cpp.o $(CLIBFILE) $(CPPLINK)

test_cpp2: makelib test_cpp2.o
	$(CXX) -o test_cpp2 test_cpp2.o $(CPPLIBFILE) $(CPPLINK)

test_f90: makelib test_f90.o
	$(F90) -o test_f90 test_f90.o $(F90LIBFILE) $(F90LINK)

test_c_speed: makelib test_c_speed.o
	$(CC) -o test_c_speed test_c_speed.o $(CLIBFILE) $(CLINK)


test_c.o: test.c
	$(CC) $(CFLAGS) -I$(CLIBDIR) $< -o $@

test_cpp.o: test.cpp
	$(CXX) $(CPPFLAGS) -I$(CLIBDIR) $< -o $@

test_cpp2.o: test2.cpp
	$(CXX) $(CPPFLAGS) -I$(CPPLIBDIR) -c $< -o $@

test_f90.o: test.fpp
	$(F90) $(F90FLAGS) -I$(F90LIBDIR) $< -o $@

test_c_speed.o: test_speed.c
	$(CC) $(CFLAGS) -I$(CLIBDIR) $< -o $@


makelib:
	cd $(CLIBDIR);   make
	cd $(CPPLIBDIR); make
	cd $(F90LIBDIR); make

clean:
	rm -f *.o *~ *.bak test_c test_cpp test_cpp2 test_f90 test_c_speed

all_clean:
	cd $(CLIBDIR);   make clean
	cd $(CPPLIBDIR); make clean
	cd $(F90LIBDIR); make clean
	rm -f *.o *~ *.bak test_c test_cpp test_cpp2 test_f90 test_c_speed
