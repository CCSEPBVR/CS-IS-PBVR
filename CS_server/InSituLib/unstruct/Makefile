include ../../pbvr.conf
include ../../arch/${PBVR_MACHINE}

ifeq ($(ISPBVR_SUPPORT_VTK), 1)
ifeq ($(PBVR_MAKE_KVSML_COMVERTER),0)
$(error PBVR_MAKE_KVSML_CONVERTER is set to 0. Compilation aborted.)
endif
endif
#$(info [DEBUG] VTK_PATH = $(VTK_PATH))
#$(info [DEBUG] VTK_INC_PATH = $(VTK_INC_PATH))
PROG=libInSituPBVR.a

VISMODULE_SOURCE_DIR        = ../../VisModule
KVS_SOURCE_DIR        = ../../KVS/Source
#TRANSFER_FUNCTION_DIR = ./TFS
FUNCTION_PARSER_DIR   = ../../FunctionParser
#IMPORT_DIR            = Importer/Importer
KVSML_CONVERTER_SOURCE_DIR = ../../KVSMLConverter/Source
IMPORT_DIR            = ${KVSML_CONVERTER_SOURCE_DIR}/Importer/
COMMON_DIR            = ../../Common


EXTERNAL_OBJS = ${VISMODULE_SOURCE_DIR}/../Common/Serializer.o \
				${VISMODULE_SOURCE_DIR}/../Common/VariableRange.o \
				${VISMODULE_SOURCE_DIR}/../Common/ParticleTransferProtocol.o 

ifeq ($(ISPBVR_SUPPORT_VTK), 1)
EXTERNAL_OBJS += ${KVSML_CONVERTER_SOURCE_DIR}/Release/Importer/VtkImport.o 
endif

LOCAL_OBJS = TransferFunction.o \
       ObjectBase.o \
       TransferFunctionSynthesizer.o \
       ParamInfo.o \
       VolumeObjectBase.o 
	

#ifeq ($(ISPBVR_SUPPORT_VTK), 1)
#LOCAL_OBJS +=	   VtkImport.o 
#endif

LOCAL_OBJS +=GlyphSeed.o \
	   GlyphProperty.o \
	   KVSMLObjectGlyph.o \
	   KVSMLObjectPlotOverLine.o \
	   PlotOverLine.o \
	   PlotOverLineProperty.o \
	   CellByCellRejectionSampling.o \
	   CellByCellHistogram.o \
       kvs_wrapper.o

OBJS = ${LOCAL_OBJS} ${EXTERNAL_OBJS}
#${VISMODULE_SOURCE_DIR}/../Common/Serializer.o:
#	@echo "Using prebuilt Serializer.o"

VPATH=${TRANSFER_FUNCTION_DIR}:${IMPORT_DIR}:${VISMODULE_SOURCE_DIR}: \
	${VISMODULE_SOURCE_DIR}/Visualization/Mapper: \
	${VISMODULE_SOURCE_DIR}/Visualization/Object: \
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

#ifeq ($(VTK_PATH), true)
ifeq ($(ISPBVR_SUPPORT_VTK), 1)
CXXFLAGS += -std=c++17 -DVTK
#CXXFLAGS += -I${VTK_INC_PATH} -I${KVSML_CONVERTER_SOURCE_DIR}/FileFormat -I${KVSML_CONVERTER_SOURCE_DIR}/Importer -I./Filter
#CXXFLAGS += -L${VTK_LIB_PATH} -lvtkFiltersExtraction-9.2 -lvtkIOXML-9.2 -lvtkIOXMLParser-9.2 -lvtkexpat-9.2 -lvtkParallelCore-9.2 -lvtkIOLegacy-9.2 -lvtkIOCore-9.2
CXXFLAGS += -I${KVS_DIR}/include -I${VTK_INCLUDE_PATH} -I${KVSML_CONVERTER_SOURCE_DIR} -I${KVSML_CONVERTER_SOURCE_DIR}/Importer -I./Filter -I${COMMON_DIR}
CXXFLAGS += -L${KVS_DIR}/lib -lkvsCore -L${VTK_LIB_PATH} -lvtkFiltersExtraction-9.2 -lvtkIOXML-9.2 -lvtkIOXMLParser-9.2 -lvtkexpat-9.2 -lvtkParallelCore-9.2 -lvtkIOLegacy-9.2 -lvtkIOCore-9.2 \
    -lvtkdoubleconversion-9.2 -lvtklz4-9.2 -lvtklzma-9.2 -lvtkFiltersStatistics-9.2 -lvtkFiltersCore-9.2 -lvtkCommonExecutionModel-9.2  \
    -lvtkzlib-9.2 -lvtkCommonColor-9.2 -lvtkfmt-9.2 -lvtkCommonDataModel-9.2 -lvtkpugixml-9.2 -lvtkCommonSystem-9.2 -lvtkCommonMisc-9.2 -lvtkCommonTransforms-9.2 \
    -lvtkCommonMath-9.2 -lm -lvtkCommonCore-9.2 -lvtkloguru-9.2 -lvtksys-9.2
endif

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
