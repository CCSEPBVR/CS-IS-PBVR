#=============================================================================
#  Sub directory.
#============================================================================
include ../pbvr.conf
include ../arch/$(PBVR_MACHINE)

NTL=ntl-5.5.2
RANDOM=random/c++

ifdef NTL_CC
    CC_STRING     ="CC=$(NTL_CC)"
endif

ifdef NTL_CXX
    CXX_STRING    ="CXX=$(NTL_CXX)"
endif

ifdef NTL_CFLAGS
    CFLAGS_STRING ="CFLAGS=$(NTL_CFLAGS)"
endif

all:
	cd $(NTL)/src && sh configure $(CC_STRING) $(CXX_STRING) $(CFLAGS_STRING)
	if [ ! `echo $(PBVR_MACHINE) | grep fujitsu` ]; \
	then \
	    cd $(NTL)/src && make && mv ntl.a libntl.a; \
	fi
	cd $(RANDOM) && make

clean:
	if [ ! `echo $(PBVR_MACHINE) | grep fujitsu` ]; \
	then \
	    cd $(NTL)/src && make clean; \
	fi
	cd $(RANDOM) && make clean
all_clean:
	if [ ! `echo $(PBVR_MACHINE) | grep fujitsu` ]; \
	then \
	    cd $(NTL)/src && rm -f libntl.a &&  make clobber; \
	fi
	cd $(RANDOM) && make all_clean
