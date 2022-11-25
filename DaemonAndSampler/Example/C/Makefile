.PHONY: all


SUBDIRS += Hydrogen_struct
SUBDIRS += Hydrogen_unstruct
SUBDIRS += Hydrogen_AMR

all:
	for i in $(SUBDIRS); do \
	$(MAKE) -C $$i; done

clean:
	for i in $(SUBDIRS); do \
	$(MAKE) -C $$i clean; done
