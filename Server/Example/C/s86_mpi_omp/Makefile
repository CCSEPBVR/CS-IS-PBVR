PBVR_DIR = ../../..

include $(PBVR_DIR)/pbvr.conf
include $(PBVR_DIR)/arch/$(PBVR_MACHINE)

MPICXX ?= mpic++
MPICXXFLAGS ?= $(CXXFLAGS)
MPILD ?= $(MPICXX)
MPILDFLAGS ?= $(LDFLAGS)

export MPICXX MPICXXFLAGS MPILD MPILDFLAGS

.PHONY: all

SUBDIRS += Hydrogen_struct
SUBDIRS += Hydrogen_unstruct
SUBDIRS += Hydrogen_unstruct_tetra
SUBDIRS += Hydrogen_unstruct_prism
SUBDIRS += Hydrogen_unstruct_pyramid
SUBDIRS += Hydrogen_unstruct_quadtetra
SUBDIRS += Hydrogen_unstruct_quadhexa
# SUBDIRS += Hydrogen_AMR
# アンサンブル用は ensemble/ にまとめた。make -C ensemble で建てる。

all:
	for i in $(SUBDIRS); do \
	$(MAKE) -C $$i; done

clean:
	for i in $(SUBDIRS); do \
	$(MAKE) -C $$i clean; done
