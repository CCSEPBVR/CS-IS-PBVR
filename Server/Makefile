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

#=============================================================================
#  Definition.
#=============================================================================
DEFINITIONS :=

ifndef MPICXX
DEFINITIONS += -DCPU_VER
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
INCLUDE_PATH := -I./Utils -I./App
LIBRARY_PATH :=
LINK_LIBRARY :=

#=============================================================================
# KVS
#=============================================================================
INCLUDE_PATH += -I$(KVS_DIR)/include
LIBRARY_PATH += -L$(KVS_DIR)/lib
LINK_LIBRARY += -lkvsCore

#=============================================================================
# uWebSockets
#=============================================================================
INCLUDE_PATH += -I$(KVS_UWS_DIR)/src -I$(KVS_UWS_DIR)/uSockets/src
LIBRARY_PATH += -L/opt/homebrew/opt/openssl@3/lib
LINK_LIBRARY += $(KVS_UWS_DIR)/uSockets/uSockets.a -lssl -lcrypto -lz

SUBDIRS := Utils
SUBDIRS += App

#=========================================================
# VPATH（検索パス）設定
#=========================================================
vpath %.cpp $(SUBDIRS)
vpath %.h   $(SUBDIRS)

OUTDIR := Release
TARGET := pbvr_server

SRCS := $(foreach d, $(SUBDIRS), $(wildcard $(d)/*.cpp))
OBJS := $(patsubst %.cpp, $(OUTDIR)/%.o, $(notdir $(SRCS)))

all: $(TARGET)

$(TARGET): $(OBJS)
	$(LD) $(LDFLAGS) $(DEFINITIONS) $(LIBRARY_PATH) $(LINK_LIBRARY) $^ -o $@

$(OUTDIR)/%.o: %.cpp %.h
	$(MKDIR) $(OUTDIR)
	$(CPP) $(CPPFLAGS) $(DEFINITIONS) $(INCLUDE_PATH) -c $< -o $@

$(OUTDIR)/%.o: %.cpp
	$(MKDIR) $(OUTDIR)
	$(CPP) $(CPPFLAGS) $(DEFINITIONS) $(INCLUDE_PATH) -c $< -o $@

clean:
	$(RMDIR) `find $(OUTDIR) -name "*.o"`
	$(RM) $(TARGET)