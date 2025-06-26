#=============================================================================
#  Sub directory.
#============================================================================
include pbvr.conf
include arch/$(PBVR_MACHINE)
include VisModule/Makefile.def

SUBDIRS += InsituLib
SUBDIRS := FunctionParser
SUBDIRS += Common
SUBDIRS += VisModule

INCLUDE_PATH := -I. -I./VisModule -I./FunctionParser -I./Common
LIBRARY_PATH := -L./VisModule/Release -L./FunctionParser -L./Common
LINK_LIBRARY := -lpthread -lvisModule -lpbvrFunc -lpbvrCommon

ifdef KVS_DIR
include $(KVS_DIR)/kvs.conf
ifeq "$(KVS_SUPPORT_EXTENDED_FILE_FORMAT)" "1"
INCLUDE_PATH += -I$(KVS_DIR)/include -I$(VTK_INCLUDE_PATH)
LIBRARY_PATH += -L$(KVS_DIR)/lib -L$(VTK_LIB_PATH)
LINK_LIBRARY += -lkvsCore -lkvsSupportExtendedFileFormat \
-lvtksys-$(VTK_VERSION) \
-lvtkCommonCore-$(VTK_VERSION) \
-lvtkCommonExecutionModel-$(VTK_VERSION) \
-lvtkCommonDataModel-$(VTK_VERSION) \
-lvtkFiltersCore-$(VTK_VERSION) \
-lvtkFiltersExtraction-$(VTK_VERSION) \
-lvtkFiltersGeneral-$(VTK_VERSION) \
-lvtkFiltersGeometry-$(VTK_VERSION) \
-lvtkFiltersParallel-$(VTK_VERSION) \
-lvtkIOCore-$(VTK_VERSION) \
-lvtkIOGeometry-$(VTK_VERSION) \
-lvtkIOCGNSReader-$(VTK_VERSION) \
-lvtkIOEnSight-$(VTK_VERSION) \
-lvtkIOParallel-$(VTK_VERSION) \
-lvtkIOLegacy-$(VTK_VERSION) \
-lvtkIOXML-$(VTK_VERSION) \
-lvtkIOXMLParser-$(VTK_VERSION)
endif
endif

ifeq "$(PBVR_MAKE_FILTER)" "1"
    SUBDIRS += Filter
endif

ifeq "$(PBVR_MAKE_KVSML_COMVERTER)" "1"
	SUBDIRS += KVSMLConverter
endif

all:
	for i in $(SUBDIRS); do \
	$(MAKE) -C $$i; done
	$(CPP) main.cpp $(CPPFLAGS) $(DEFINITIONS) $(INCLUDE_PATH) $(LDFLAGS) $(LIBRARY_PATH) $(LINK_LIBRARY) -o pbvr_server
clean:
	for i in $(SUBDIRS); do \
	$(MAKE) -C $$i clean; done
	$(RM) pbvr_server

all_clean:
	for i in $(SUBDIRS); do \
	$(MAKE) -C $$i all_clean; done
	$(RM) pbvr_server

