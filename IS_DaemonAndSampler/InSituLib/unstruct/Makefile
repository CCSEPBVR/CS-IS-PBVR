include ../../pbvr.conf
include ../../arch/${PBVR_MACHINE}

PROG=libInSituPBVR.a

KVS_SOURCE_DIR        = ../../KVS/Source
TRANSFER_FUNCTION_DIR = ./TFS
FUNCTION_PARSER_DIR   = ../../FunctionParser
IMPORT_DIR            = Importer/Importer

OBJS = Serializer.o \
       TransferFunction.o \
       ObjectBase.o \
       TransferFunctionSynthesizer.o \
       ParamInfo.o \
       VolumeObjectBase.o \
	   VtkImport.o \
	   GlyphGenerator.o \
	   GlyphProperty.o \
	   KVSMLObjectGlyph.o \
       kvs_wrapper.o

VPATH=${TRANSFER_FUNCTION_DIR}:${IMPORT_DIR}

ifdef MPICXX
    CXX      = ${MPICXX}
endif

ifdef MPICXXFLAGS
    CXXFLAGS = ${MPICXXFLAGS}
endif

CXXFLAGS += -I../shared
CXXFLAGS += -I${KVS_SOURCE_DIR}
CXXFLAGS += -I${FUNCTION_PARSER_DIR}
CXXFLAGS += -w -DNO_CLIENT

CXXFLAGS += -DVTK
CXXFLAGS += -I/home/center/viztools/VTK-9.2.2/include/vtk-9.2 -I./VTK -I./Importer -I./Filter
CXXFLAGS += -L/home/center/viztools/VTK-9.2.2/lib64 -lvtkFiltersExtraction-9.2 -lvtkIOXML-9.2 -lvtkIOXMLParser-9.2 -lvtkexpat-9.2 -lvtkParallelCore-9.2 -lvtkIOLegacy-9.2 -lvtkIOCore-9.2 \
    -lvtkdoubleconversion-9.2 -lvtklz4-9.2 -lvtklzma-9.2 -lvtkFiltersStatistics-9.2 -lvtkFiltersCore-9.2 -lvtkCommonExecutionModel-9.2  \
    -lvtkzlib-9.2 -lvtkCommonColor-9.2 -lvtkfmt-9.2 -lvtkCommonDataModel-9.2 -lvtkpugixml-9.2 -lvtkCommonSystem-9.2 -lvtkCommonMisc-9.2 -lvtkCommonTransforms-9.2 \
    -lvtkCommonMath-9.2 -lm -lvtkCommonCore-9.2 -lvtkloguru-9.2 -lvtksys-9.2

all : ${PROG}

${PROG} : ${OBJS}
	${AR} ${ARFLAGS} $@ ${OBJS}

%.o : %.c
	${CC} -c ${CCFLAGS} $<

%.o : %.cpp
	${CXX} -c ${CXXFLAGS} $<

clean :
	${RM} -f ${OBJS}  *.o core *.bmp *.linkinfo *.vtk

all_clean :
	${RM} -f ${PROG} ${OBJS}  *.o core *.bmp *.linkinfo *.vtk
