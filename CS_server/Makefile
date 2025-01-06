#=============================================================================
#  Sub directory.
#============================================================================
include pbvr.conf
include arch/$(PBVR_MACHINE)

SUBDIRS := FunctionParser
SUBDIRS += Common
SUBDIRS += VisModule

ifeq "$(PBVR_MAKE_FILTER)" "1"
    SUBDIRS += Filter
endif

ifeq "$(PBVR_MAKE_KVSML_COMVERTER)" "1"
	SUBDIRS += KVSMLConverter
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

