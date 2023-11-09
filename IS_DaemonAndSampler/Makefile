#=============================================================================
#  Sub directory.
#============================================================================
include pbvr.conf
include arch/$(PBVR_MACHINE)

SUBDIR_KVS := KVS
SUBDIRS := FunctionParser
SUBDIRS += Common

ifeq "$(PBVR_MAKE_CLIENT)" "1"
    SUBDIRS += glui/src
    SUBDIRS += Client
else
    SUBDIRS += Daemon
    SUBDIRS += InSituLib/struct
    SUBDIRS += InSituLib/unstruct
    SUBDIRS += InSituLib/AMR
endif

all:
	$(MAKE) -C $(SUBDIR_KVS) all_clean
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

