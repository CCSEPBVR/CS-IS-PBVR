include ../../pbvr.conf
include ../../arch/${PBVR_MACHINE}

#ifeq ($(VTK_PATH), true)
#include ../../VTK.path	
#endif

PROG=libInSituPBVR.a

VISMODULE_SOURCE_DIR        = ../../VisModule
KVS_SOURCE_DIR        = ../../KVS/Source
#TRANSFER_FUNCTION_DIR = ./TFS
FUNCTION_PARSER_DIR   = ../../FunctionParser
#IMPORT_DIR            = Importer/Importer


EXTERNAL_OBJS = ${VISMODULE_SOURCE_DIR}/../Common/Serializer.o \
				${VISMODULE_SOURCE_DIR}/../Common/VariableRange.o

LOCAL_OBJS = TransferFunction.o \
       ObjectBase.o \
       TransferFunctionSynthesizer.o \
       TrilinearInterpolator.o \
       ParamInfo.o \
       VolumeObjectBase.o 
	

#ifeq ($(VTK_PATH), true)
#LOCAL_OBJS +=	   VtkImport.o 
#endif

LOCAL_OBJS +=GlyphGenerator.o \
	   GlyphProperty.o \
	   KVSMLObjectGlyph.o \
	   KVSMLObjectPlotOverLine.o \
	   PlotOverLine.o \
	   PlotOverLineProperty.o \
       kvs_wrapper.o

OBJS = ${LOCAL_OBJS} ${EXTERNAL_OBJS}
#${VISMODULE_SOURCE_DIR}/../Common/Serializer.o:
#	@echo "Using prebuilt Serializer.o"

VPATH=${TRANSFER_FUNCTION_DIR}:${IMPORT_DIR}:${VISMODULE_SOURCE_DIR}: \
	${VISMODULE_SOURCE_DIR}/Visualization/Mapper: \
	${VISMODULE_SOURCE_DIR}/Visualization/Object: \
	${VISMODULE_SOURCE_DIR}/Visualization/Filter: \
	${VISMODULE_SOURCE_DIR}/DaemonAndSampler/Daemon:\
	${VISMODULE_SOURCE_DIR}/FileFormat:\
	${VISMODULE_SOURCE_DIR}/../Common: \
	${VISMODULE_SOURCE_DIR}/Visualization/Property

ifdef MPICXX
    CXX      = ${MPICXX}
endif

ifdef MPICXXFLAGS
    CXXFLAGS = ${MPICXXFLAGS}
endif

CXXFLAGS += -I../shared
CXXFLAGS += -I${KVS_SOURCE_DIR}
CXXFLAGS += -I${VISMODULE_SOURCE_DIR}
CXXFLAGS += -I${FUNCTION_PARSER_DIR}
CXXFLAGS += -w -DNO_CLIENT

all : ${PROG}

${PROG} : ${OBJS}
	${AR} ${ARFLAGS} $@ ${OBJS}

%.o : %.c
	${CC} -c ${CCFLAGS} $<

%.o : %.cpp
	${CXX} -c ${CXXFLAGS} $<

${EXTERNAL_OBJS}:
	@echo "Using prebuilt Serializer.o"

clean :
	${RM} -f ${LOCAL_OBJS}  *.o core *.bmp *.linkinfo *.vtk

all_clean :
	${RM} -f ${PROG} ${OBJS}  *.o core *.bmp *.linkinfo *.vtk
