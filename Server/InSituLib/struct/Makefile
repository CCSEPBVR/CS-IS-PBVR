include ../../pbvr.conf
include ../../arch/${PBVR_MACHINE}

VISMODULE_SOURCE_DIR = ../../VisModule
FUNCTION_PARSER_DIR  = ../../FunctionParser

OBJS  = \
../shared/kvs_wrapper_common.o \
../shared/EnsembleStatisticOutput.o \
../shared/EnsembleParticleGenerator.o \
../shared/ChainRuleNormal.o \
../shared/VertexTFGradient.o \
../shared/EnsembleCellHistogram.o \
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
CXXFLAGS += -DPBVR_SIMD_CHAINRULE
CXXFLAGS += -DPBVR_UNIFORM_FISSION2

all : ${PROG}

${PROG} : ${OBJS}
	${AR} ${ARFLAGS} $@ ${OBJS}

../shared/%.o : ../shared/%.cpp
	${CXX} -c ${CXXFLAGS} $< -o $@

%.o : %.c
	${CC} -c ${CCFLAGS} $<

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
