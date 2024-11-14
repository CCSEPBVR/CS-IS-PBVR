.PHONY: all


SUBDIRS += Hydrogen_struct
SUBDIRS += Hydrogen_unstruct
SUBDIRS += Hydrogen_unstruct_tetra
SUBDIRS += Hydrogen_unstruct_prism
SUBDIRS += Hydrogen_unstruct_pyramid
SUBDIRS += Hydrogen_unstruct_quadtetra
SUBDIRS += Hydrogen_unstruct_quadhexa
SUBDIRS += Hydrogen_AMR
SUBDIRS += Hydrogen_unstruct_tetra
SUBDIRS += Hydrogen_unstruct_pyramid
SUBDIRS += Hydrogen_unstruct_prism
SUBDIRS += Hydrogen_unstruct_quadhexa
SUBDIRS += Hydrogen_unstruct_quadtetra

all:
	for i in $(SUBDIRS); do \
	$(MAKE) -C $$i; done

clean:
	for i in $(SUBDIRS); do \
	$(MAKE) -C $$i clean; done
