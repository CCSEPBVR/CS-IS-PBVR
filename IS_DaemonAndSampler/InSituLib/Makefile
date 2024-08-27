.PHONY: all


SUBDIRS += struct
SUBDIRS += unstruct
SUBDIRS += AMR

all:
	for i in $(SUBDIRS); do \
	$(MAKE) -C $$i; done

clean:
	for i in $(SUBDIRS); do \
	$(MAKE) -C $$i clean; done

