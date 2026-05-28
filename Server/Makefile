#=============================================================================
#  Include.
#=============================================================================
include pbvr.conf
include arch/$(PBVR_MACHINE)

#=============================================================================
#  Command macro.
#=============================================================================
CPP    := $(CXX)
LD     := $(CPP)

ifdef MPICXX
CPP    := $(MPICXX)
endif

ifdef MPILD
LD     := $(MPILD)
endif

MKDIR := @mkdir -pv
RM    := @rm -fv
RMDIR := @rm -Rfv

#=============================================================================
#  Compiler option.
#=============================================================================
CPPFLAGS += $(CXXFLAGS)

ifdef MPICXXFLAGS
CPPFLAGS += $(MPICXXFLAGS)
endif

# ヘッダファイルを修正した際にも自動で反映される
CPPFLAGS += -MMD -MP

#=============================================================================
#  Definition.
#=============================================================================
DEFINITIONS :=

ifndef MPICXX
DEFINITIONS += -DCPU_VER
endif

PBVR_KVS_INSTALL_DIR := $(abspath ../KVS/Install)
ifneq ($(wildcard $(PBVR_KVS_INSTALL_DIR)/kvs.conf),)
include $(PBVR_KVS_INSTALL_DIR)/kvs.conf
ifeq "$(KVS_SUPPORT_EXTENDED_FILE_FORMAT)" "1"
DEFINITIONS += -DEXTEND_FILE_FORMAT
endif
endif

#=============================================================================
#  Linker option.
#=============================================================================
LDFLAGS +=

ifdef MPILDFLAGS
LDFLAGS += $(MPILDFLAGS)
endif

#=============================================================================
# INCLUDE_PATH, LIBRARY_PATH, LINK_LIBRARY
#=============================================================================
INCLUDE_PATH := -I./Utils -I./App -I./VisModule -I./FunctionParser
LIBRARY_PATH := -L./VisModule/Release -L./FunctionParser
LINK_LIBRARY := -lvisModule -lpbvrFunc

#=============================================================================
# KVS
#=============================================================================
INCLUDE_PATH += -I$(PBVR_KVS_INSTALL_DIR)/include
LIBRARY_PATH += -L$(PBVR_KVS_INSTALL_DIR)/lib
LINK_LIBRARY += -lkvsCore

#=============================================================================
# VTK
#=============================================================================
ifneq ($(wildcard $(PBVR_KVS_INSTALL_DIR)/kvs.conf),)
include $(PBVR_KVS_INSTALL_DIR)/kvs.conf
ifeq "$(KVS_SUPPORT_EXTENDED_FILE_FORMAT)" "1"
INCLUDE_PATH += -I$(VTK_INCLUDE_PATH)
LIBRARY_PATH += -L$(VTK_LIB_PATH)
LINK_LIBRARY += -lkvsSupportExtendedFileFormat \
-lvtkCommonExecutionModel-$(VTK_VERSION) \
-lvtkFiltersCore-$(VTK_VERSION) \
-lvtkFiltersExtraction-$(VTK_VERSION) \
-lvtkFiltersGeometry-$(VTK_VERSION) \
-lvtkFiltersParallel-$(VTK_VERSION) \
-lvtkCommonExecutionModel-$(VTK_VERSION) \
-lvtkFiltersCore-$(VTK_VERSION) \
-lvtkIOCore-$(VTK_VERSION) \
-lvtkIOGeometry-$(VTK_VERSION) \
-lvtkIOCGNSReader-$(VTK_VERSION) \
-lvtkIOEnSight-$(VTK_VERSION) \
-lvtkFiltersGeneral-$(VTK_VERSION) \
-lvtkIOParallel-$(VTK_VERSION) \
-lvtkIOLegacy-$(VTK_VERSION) \
-lvtkIOXML-$(VTK_VERSION) \
-lvtkCommonDataModel-$(VTK_VERSION) \
-lvtkIOXMLParser-$(VTK_VERSION) \
-lvtkCommonTransforms-$(VTK_VERSION) \
-lvtkCommonMath-$(VTK_VERSION) \
-lvtkCommonCore-$(VTK_VERSION) \
-lvtksys-$(VTK_VERSION) \
-lvtkloguru-$(VTK_VERSION) \
-lvtkIOCore-$(VTK_VERSION) \
-lvtkCommonSystem-$(VTK_VERSION) \
-lvtkCommonMisc-$(VTK_VERSION)

DEFINITIONS += -DEXTEND_FILE_FORMAT
endif
endif

#=============================================================================
# Runtime loading of shared libraries
#=============================================================================
LINK_LIBRARY += -ldl

#=============================================================================
# uWebSockets
#=============================================================================
INCLUDE_PATH += -I$(KVS_UWS_DIR)/src -I$(KVS_UWS_DIR)/uSockets/src
LIBRARY_PATH += -L/opt/homebrew/opt/openssl@3/lib
LINK_LIBRARY += $(KVS_UWS_DIR)/uSockets/uSockets.a -lssl -lcrypto -lz

#=========================================================
# VPATH（検索パス）設定
#=========================================================
SUBDIRS := Utils
SUBDIRS += App

vpath %.cpp $(SUBDIRS)
vpath %.h   $(SUBDIRS)

OUTDIR := Release
TARGET := pbvr_server

OBJS := \
	$(OUTDIR)/main.o \
	$(OUTDIR)/Server.o \
	$(OUTDIR)/ServerWorker.o \
	$(OUTDIR)/TaskSignalTransferProtocol.o

THIRD_DIRS := VisModule
THIRD_DIRS += InSituLib
THIRD_DIRS += FunctionParser

all: third $(TARGET)

third:
	for i in $(THIRD_DIRS); do \
    	$(MAKE) -C $$i; \
	done	

$(TARGET): $(OBJS)
	$(LD) $^ $(LDFLAGS) $(DEFINITIONS) $(LIBRARY_PATH) $(LINK_LIBRARY) -o $@

$(OUTDIR)/%.o: %.cpp
	$(MKDIR) $(OUTDIR)
	$(CPP) $(CPPFLAGS) $(DEFINITIONS) $(INCLUDE_PATH) -c $< -o $@

# コンパイル時に.dという依存関係リストが生成される
# どのヘッダを修正しても関係するすべての.cppが再ビルトされる
-include $(OBJS:.o=.d)

clean:
	for i in $(THIRD_DIRS); do \
	$(MAKE) -C $$i clean; done
	$(RMDIR) `find $(OUTDIR) -name "*.o"`
	$(RM) $(TARGET)
