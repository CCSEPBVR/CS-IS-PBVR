#*****************************************************************************
#  $Id: Makefile 626 2010-10-01 07:45:20Z naohisa.sakamoto $
#*****************************************************************************

#=============================================================================
#  Include.
#=============================================================================
include ../../kvs.conf
include ../../Makefile.def


#=============================================================================
#  INCLUDE_PATH, LIBRARY_PATH, LINK_LIBRARY, INSTALL_DIR.
#=============================================================================
INCLUDE_PATH := -I../../Source
LIBRARY_PATH := -L../../Source/Core/$(OUTDIR)
LINK_LIBRARY := -lkvsCore
INSTALL_DIR  := $(KVS_DIR)


#=============================================================================
#  Project name.
#=============================================================================
PROJECT_NAME := kvsmake

ifeq "$(findstring CYGWIN,$(shell uname -s))" "CYGWIN"
TARGET_EXE := $(OUTDIR)/$(PROJECT_NAME).exe
else
TARGET_EXE := $(OUTDIR)/$(PROJECT_NAME)
endif


#=============================================================================
#  Object.
#=============================================================================
OBJECTS := \
$(OUTDIR)/Argument.o \
$(OUTDIR)/WriteMakefile.o \
$(OUTDIR)/WriteQtProject.o \
$(OUTDIR)/main.o \


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
	$(INSTALL) Makefile.kvs.template $(INSTALL_DIR)/bin
	$(INSTALL) QtProject.template    $(INSTALL_DIR)/bin
	$(INSTALL_EXE) $(TARGET_EXE) $(INSTALL_DIR)/bin
