#*****************************************************************************
#  $Id: Makefile 231 2009-06-17 10:31:16Z naohisa.sakamoto $
#*****************************************************************************

#=============================================================================
#  Include.
#=============================================================================
include ../../kvs.conf
include ../../Makefile.def


#=============================================================================
#  INCLUDE_PATH, LIBRARY_PATH, LINK_LIBRARY, INSTALL_DIR.
#=============================================================================
INCLUDE_PATH := -I.. $(CUDA_INCLUDE_PATH)
LIBRARY_PATH := $(CUDA_LIBRARY_PATH)
LINK_LIBRARY := $(CUDA_LINK_LIBRARY)
INSTALL_DIR  := $(KVS_DIR)


#=============================================================================
#  Include build rule.
#=============================================================================
include BuildRule.mk


#=============================================================================
#  Project name.
#=============================================================================
TARGET_LIB := $(OUTDIR)/$(LIB_KVS_SUPPORT_CUDA)


#=============================================================================
#  Build rule.
#=============================================================================
$(TARGET_LIB): $(OBJECTS)
	$(AR) $@ $^
	$(RANLIB) $@


#=============================================================================
#  build.
#=============================================================================
build:: $(TARGET_LIB)


#=============================================================================
#  clean.
#=============================================================================
clean::
	$(RMDIR) $(OUTDIR)


#=============================================================================
#  install.
#=============================================================================
install::
	$(MKDIR) $(INSTALL_DIR)/lib
	$(INSTALL) $(TARGET_LIB) $(INSTALL_DIR)/lib
	$(RANLIB) $(INSTALL_DIR)/lib/$(LIB_KVS_SUPPORT_CUDA)
