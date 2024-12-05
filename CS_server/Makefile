#=============================================================================
#  Sub directory.
#============================================================================
include pbvr.conf
include arch/$(PBVR_MACHINE)

SUBDIRS := FunctionParser
SUBDIRS += Common
SUBDIRS += VisModule

ifeq "$(PBVR_MAKE_CLIENT)" "1"
    SUBDIRS += glui/src
    SUBDIRS += Client
endif

ifeq "$(PBVR_MAKE_FILTER)" "1"
    SUBDIRS += Filter
endif

ifeq "$(PBVR_MAKE_SERVER)" "1"
    SUBDIRS += Server
endif

all:
	for i in $(SUBDIRS); do \
	$(MAKE) -C $$i; done

clean:
	for i in $(SUBDIRS); do \
	$(MAKE) -C $$i clean; done

all_clean:
	for i in $(SUBDIRS); do \
	$(MAKE) -C $$i all_clean; done

