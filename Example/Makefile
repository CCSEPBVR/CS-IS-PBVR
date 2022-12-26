#*****************************************************************************
#  $Id: Makefile 1500 2013-04-04 07:56:40Z naohisa.sakamoto@gmail.com $
#*****************************************************************************

#=============================================================================
#  Include.
#=============================================================================
include ../Makefile.def


#=============================================================================
#  INCLUDE_PATH, LIBRARY_PATH, LINK_LIBRARY, INSTALL_DIR.
#=============================================================================
INCLUDE_PATH := -I../Source
ifdef KVS_INCLUDE_PATH
INCLUDE_PATH += -I$(KVS_INCLUDE_PATH)
endif
ifdef VTK_INCLUDE_PATH
INCLUDE_PATH += -I$(VTK_INCLUDE_PATH)
endif

LIBRARY_PATH := -L../Source/$(OUTDIR)
ifdef KVS_LIBRARY_PATH
LIBRARY_PATH += -L$(KVS_LIBRARY_PATH)
endif
ifdef VTK_LIBRARY_PATH 
LIBRARY_PATH += -L$(VTK_LIBRARY_PATH)
endif
LINK_LIBRARY := -lstdc++fs -lkvsmlconverter $(KVS_LINK_LIBRARY) $(VTK_LINK_LIBRARY)
INSTALL_DIR  := $(INSTALL_PREFIX)


#=============================================================================
#  Project name.
#=============================================================================
PROJECT_NAME := kvsml-converter

ifeq "$(findstring CYGWIN,$(shell uname -s))" "CYGWIN"
TARGET_EXE := $(OUTDIR)/$(PROJECT_NAME).exe
else
TARGET_EXE := $(OUTDIR)/$(PROJECT_NAME)
endif


#=============================================================================
#  Object.
#=============================================================================
OBJECTS := \
$(OUTDIR)/AvsExample.o \
$(OUTDIR)/CgnsExample.o \
$(OUTDIR)/EnSightExample.o \
$(OUTDIR)/Plot3dExample.o \
$(OUTDIR)/PvtsExample.o \
$(OUTDIR)/PvtuExample.o \
$(OUTDIR)/StlExample.o \
$(OUTDIR)/VtiExample.o \
$(OUTDIR)/VtmExample.o \
$(OUTDIR)/VtpExample.o \
$(OUTDIR)/VtrExample.o \
$(OUTDIR)/VtsExample.o \
$(OUTDIR)/VtuExample.o \
$(OUTDIR)/example.o


#=============================================================================
#  Build rule.
#=============================================================================
$(TARGET_EXE): $(OBJECTS)
	$(LD) $(LDFLAGS) $(LIBRARY_PATH) -o $@ $^ $(LINK_LIBRARY)

$(OUTDIR)/%.o: %.cpp %.h
	$(MKDIR) $(OUTDIR)
	$(CPP) -c $(CPPFLAGS) $(DEFINITIONS) $(INCLUDE_PATH) -o $@ $<

$(OUTDIR)/%.o: %.cpp
	$(MKDIR) $(OUTDIR)
	$(CPP) -c $(CPPFLAGS) $(DEFINITIONS) $(INCLUDE_PATH) -o $@ $<


#=============================================================================
#  build.
#=============================================================================
build: $(TARGET_EXE)


#=============================================================================
#  clean.
#=============================================================================
clean:
	$(RMDIR) $(OUTDIR)


#=============================================================================
#  install.
#=============================================================================
install:
	$(MKDIR) $(INSTALL_DIR)/bin
	$(INSTALL_EXE) $(TARGET_EXE) $(INSTALL_DIR)/bin
