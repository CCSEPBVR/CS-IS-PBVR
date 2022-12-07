#*****************************************************************************
#  $Id: Makefile 556 2010-07-15 07:39:28Z naohisa.sakamoto $
#*****************************************************************************

#=============================================================================
#  Include.
#=============================================================================
include ../../pbvr.conf
include ../../arch/$(PBVR_MACHINE)
include ../Makefile.def


#=============================================================================
#  INCLUDE_PATH, LIBRARY_PATH, LINK_LIBRARY, INSTALL_DIR.
#=============================================================================
INCLUDE_PATH :=
LIBRARY_PATH :=
LINK_LIBRARY :=
INSTALL_DIR  := ../../$(KVS_DIR_NAME)


#=============================================================================
#  Sub directory.
#=============================================================================
SUBDIRS := Core

ifeq "$(KVS_SUPPORT_GLUT)" "1"
SUBDIRS += SupportGLUT
endif

ifeq "$(KVS_SUPPORT_GLEW)" "1"
SUBDIRS += SupportGLEW
endif

ifeq "$(KVS_SUPPORT_QT)" "1"
SUBDIRS += SupportQt
endif

ifeq "$(KVS_SUPPORT_SAGE)" "1"
SUBDIRS += SupportSAGE
endif

ifeq "$(KVS_SUPPORT_OPENCV)" "1"
SUBDIRS += SupportOpenCV
endif

ifeq "$(KVS_SUPPORT_CUDA)" "1"
SUBDIRS += SupportCUDA
endif

ifeq "$(KVS_SUPPORT_OPENCABIN)" "1"
SUBDIRS += SupportOpenCABIN
endif


#=============================================================================
#  build.
#=============================================================================
build:
	for i in $(SUBDIRS); do \
	$(MAKE) -C $$i $(MFLAGS) $(MAKEOVERRIDES) build; done


#=============================================================================
#  clean.
#=============================================================================
clean:
	for i in $(SUBDIRS); do \
	$(MAKE) -C $$i $(MFLAGS) $(MAKEOVERRIDES) clean; done

all_clean:
	for i in $(SUBDIRS); do \
	$(MAKE) -C $$i $(MFLAGS) $(MAKEOVERRIDES) all_clean; done

#=============================================================================
#  install.
#=============================================================================
install:
	$(MKDIR) $(INSTALL_DIR)/include/kvs
	$(INSTALL) kvsCore $(INSTALL_DIR)/include
	$(INSTALL) $(shell find kvs -maxdepth 1 -type f) $(INSTALL_DIR)/include/kvs
ifeq "$(KVS_SUPPORT_GLUT)" "1"
	$(MKDIR) $(INSTALL_DIR)/include/kvs/glut
	$(INSTALL) kvsSupportGLUT $(INSTALL_DIR)/include
	$(INSTALL) $(shell find kvs/glut -maxdepth 1 -type f) $(INSTALL_DIR)/include/kvs/glut
endif
ifeq "$(KVS_SUPPORT_GLEW)" "1"
	$(MKDIR) $(INSTALL_DIR)/include/kvs/glew
	$(INSTALL) kvsSupportGLEW $(INSTALL_DIR)/include
	$(INSTALL) $(shell find kvs/glew -maxdepth 1 -type f) $(INSTALL_DIR)/include/kvs/glew
endif
ifeq "$(KVS_SUPPORT_QT)" "1"
	$(MKDIR) $(INSTALL_DIR)/include/kvs/qt
	$(INSTALL) kvsSupportQt $(INSTALL_DIR)/include
	$(INSTALL) $(shell find kvs/qt -maxdepth 1 -type f) $(INSTALL_DIR)/include/kvs/qt
endif
ifeq "$(KVS_SUPPORT_OPENCV)" "1"
	$(MKDIR) $(INSTALL_DIR)/include/kvs/opencv
	$(INSTALL) kvsSupportOpenCV $(INSTALL_DIR)/include
	$(INSTALL) $(shell find kvs/opencv -maxdepth 1 -type f) $(INSTALL_DIR)/include/kvs/opencv
endif
ifeq "$(KVS_SUPPORT_OPENCABIN)" "1"
	$(MKDIR) $(INSTALL_DIR)/include/kvs/opencabin
	$(INSTALL) kvsSupportOpenCABIN $(INSTALL_DIR)/include
	$(INSTALL) $(shell find kvs/opencabin -maxdepth 1 -type f) $(INSTALL_DIR)/include/kvs/opencabin
endif
ifeq "$(KVS_SUPPORT_CUDA)" "1"
	$(MKDIR) $(INSTALL_DIR)/include/kvs/cuda
	$(INSTALL) kvsSupportCUDA $(INSTALL_DIR)/include
	$(INSTALL) $(shell find kvs/cuda -maxdepth 1 -type f) $(INSTALL_DIR)/include/kvs/cuda
endif
ifeq "$(KVS_SUPPORT_SAGE)" "1"
	$(MKDIR) $(INSTALL_DIR)/include/kvs/sage
	$(INSTALL) kvsSupportSAGE $(INSTALL_DIR)/include
	$(INSTALL) $(shell find kvs/sage -maxdepth 1 -type f) $(INSTALL_DIR)/include/kvs/sage
endif
	for i in $(SUBDIRS); do \
	$(MAKE) -C $$i $(MFLAGS) $(MAKEOVERRIDES) install; done
