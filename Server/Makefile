#=============================================================================
#  Sub directory.
#============================================================================
include pbvr.conf
include arch/$(PBVR_MACHINE)

SUBDIR_KVS := KVS
SUBDIRS := FunctionParser
SUBDIRS += Common

ifeq "$(PBVR_SUPPORT_KMATH)" "1"
    SUBDIRS += KMATH
endif

ifeq "$(PBVR_MAKE_CLIENT)" "1"
    SUBDIRS += glui/src
    SUBDIRS += Client
endif

ifeq "$(PBVR_MAKE_SERVER)" "1"
    SUBDIRS += PbvrFilter
    SUBDIRS += Server
endif

all:
	$(MAKE) -C $(SUBDIR_KVS)
	for i in $(SUBDIRS); do \
	$(MAKE) -C $$i; done

clean:
	$(MAKE) -C $(SUBDIR_KVS) clean
	for i in $(SUBDIRS); do \
	$(MAKE) -C $$i clean; done

all_clean:
	$(MAKE) -C $(SUBDIR_KVS) all_clean
	for i in $(SUBDIRS); do \
	$(MAKE) -C $$i all_clean; done

