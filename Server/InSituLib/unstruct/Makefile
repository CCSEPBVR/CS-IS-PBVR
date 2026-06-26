include ../../pbvr.conf
include ../../arch/${PBVR_MACHINE}

VISMODULE_SOURCE_DIR       = ../../VisModule
FUNCTION_PARSER_DIR        = ../../FunctionParser

OBJS  = \
../shared/kvs_wrapper_common.o \
ChainRuleNormal.o \
EnsembleCellHistogram.o \
kvs_wrapper.o

PROG = libInSituPBVR.a

ifdef MPICXX
    CXX      = ${MPICXX}
endif

ifdef MPICXXFLAGS
    CXXFLAGS = ${MPICXXFLAGS}
else
	CXXFLAGS += -DCPU_VER
endif

CXXFLAGS += -I../../Utils
CXXFLAGS += -I../../App
CXXFLAGS += -I../shared
CXXFLAGS += -I${VISMODULE_SOURCE_DIR}
CXXFLAGS += -I${FUNCTION_PARSER_DIR}
PBVR_KVS_INSTALL_DIR ?= $(abspath ../../../KVS/Install)
CXXFLAGS += -I$(PBVR_KVS_INSTALL_DIR)/include
CXXFLAGS += -w -DNO_CLIENT
CXXFLAGS += -DENABLE_ENSEMBLE_TIMER
CXXFLAGS += -DENABLE_HEX_TET_VOLUME
CXXFLAGS += -DPBVR_SIMD_CHAINRULE

ifneq ($(wildcard $(PBVR_KVS_INSTALL_DIR)/kvs.conf),)
include $(PBVR_KVS_INSTALL_DIR)/kvs.conf
ifeq "$(KVS_SUPPORT_EXTENDED_FILE_FORMAT)" "1"
CXXFLAGS += -std=c++17 -DEXTEND_FILE_FORMAT
CXXFLAGS += \
-I$(PBVR_KVS_INSTALL_DIR)/include \
-I$(VTK_INCLUDE_PATH) \
-I./Filter
@echo "INFO:KVS_SUPPORT_EXTENDED_FILE_FORMAT=1. OpenFOAM is available."	
else
@echo "INFO:KVS_SUPPORT_EXTENDED_FILE_FORMAT=0. OpenFOAM is not available."	
endif
else
@echo "INFO:KVS install is not found. OpenFOAM is not available."
endif

all : ${PROG}

${PROG} : ${OBJS}
	${AR} ${ARFLAGS} $@ ${OBJS}

%.o : %.c
	${CC} -c ${CCFLAGS} $<

../shared/%.o : ../shared/%.cpp
	${CXX} -c ${CXXFLAGS} $< -o $@

%.o : %.cpp
	${CXX} -c ${CXXFLAGS} $<

clean :
	${RM} -f ${OBJS} ${OBJS:.o=.d}

all_clean :
	${RM} -f ${PROG} ${OBJS} ${OBJS:.o=.d}

# --- dependency tracking: rebuild on header or flag (Makefile/arch) changes ---
CXXFLAGS += -MMD -MP
${OBJS} : Makefile ../../arch/${PBVR_MACHINE}
-include ${OBJS:.o=.d}
