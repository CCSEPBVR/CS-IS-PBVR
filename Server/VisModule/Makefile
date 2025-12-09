#*****************************************************************************
#  $Id: Makefile 62 2009-03-19 06:58:44Z naohisa.sakamoto $
#*****************************************************************************

#=============================================================================
#  Include.
#=============================================================================
include ../pbvr.conf
include ../arch/$(PBVR_MACHINE)
include ./Makefile.def


#=============================================================================
#  INCLUDE_PATH, LIBRARY_PATH, LINK_LIBRARY, INSTALL_DIR.
#=============================================================================
INCLUDE_PATH := -I. -I../FunctionParser

ifdef KVS_DIR
include $(KVS_DIR)/kvs.conf
ifeq "$(KVS_SUPPORT_EXTENDED_FILE_FORMAT)" "1"
INCLUDE_PATH += -I$(KVS_DIR)/include -I$(VTK_INCLUDE_PATH)
endif
endif

INSTALL_DIR  := ../../../$(VIS_MODULE_DIR_NAME)

#=============================================================================
#  Include build rule.
#=============================================================================
include BuildRuleServerOnly.mk

#=============================================================================
#  Project name.
#=============================================================================
TARGET_LIB := $(OUTDIR)/$(LIB_VIS_MODULE_CORE)
#PROJECT_NAME := pbvr_server

ifeq "$(findstring CYGWIN,$(shell uname -s))" "CYGWIN"
TARGET_EXE := ../$(PROJECT_NAME).exe
else
TARGET_EXE := ../$(PROJECT_NAME)
endif

#=============================================================================
#  Build rule.
#=============================================================================
$(TARGET_LIB): $(OBJECTS)
	$(AR) $@ $^
	$(RANLIB) $@

#$(TARGET_EXE): $(OBJECTS)
#	$(LD) $(LDFLAGS) $(LIBRARY_PATH) -o $@ $^ $(LINK_LIBRARY)

$(OUTDIR)/%.o: %.cpp %.h
	$(MKDIR) $(OUTDIR)
	$(CPP) -c $(CPPFLAGS) $(DEFINITIONS) $(INCLUDE_PATH) -o $@ $<

$(OUTDIR)/%.o: %.cpp
	$(MKDIR) $(OUTDIR)
	$(CPP) -c $(CPPFLAGS) $(DEFINITIONS) $(INCLUDE_PATH) -o $@ $<

#=============================================================================
#  build.
#=============================================================================
build:: $(TARGET_LIB)
#build:: $(TARGET_EXE)


#=============================================================================
#  clean.
#=============================================================================
clean::
	$(RMDIR) `find $(OUTDIR) -name "*.o"`
	# $(RM) $(TARGET_EXE)

all_clean::
	$(RMDIR) $(OUTDIR)
	# $(RM) $(TARGET_EXE)

#=============================================================================
#  install.
#=============================================================================
install::
	$(MKDIR) $(INSTALL_DIR)/lib
	$(INSTALL) $(TARGET_LIB) $(INSTALL_DIR)/lib
	$(RANLIB) $(INSTALL_DIR)/lib/$(LIB_VIS_MODULE_CORE)
