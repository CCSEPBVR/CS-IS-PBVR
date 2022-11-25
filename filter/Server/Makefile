include ../pbvr.conf
include ../arch/${PBVR_MACHINE}

TARGET        = pbvr_server
FUNC_DIR      = ../FunctionParser
COMMON_DIR    = ../Common/
KVS_SOURCE_DIR= ../KVS/Source
PBVRFILTER_DIR = ../PbvrFilter
# CXXFLAGS +=-std=c++11

ifdef MPICXX
    CXX     = ${MPICXX}
endif

ifdef MPICXXFLAGS
    CXXFLAGS += ${MPICXXFLAGS}
    ifeq "$(PBVR_SUPPORT_KMATH)" "1"
        CXXFLAGS += -DKMATH -I../KMATH/random/c++/ -I../KMATH/ntl-5.5.2/include
    endif
endif

ifdef MPILD
    LD      = ${MPILD}
endif

ifdef MPILDFLAGS
    LDFLAGS = ${MPILDFLAGS}
    ifeq "$(PBVR_SUPPORT_KMATH)" "1"
        LDFLAGS += -L../KMATH/random/c++/
        ifneq (,$(findstring fujitsu,$(PBVR_MACHINE)))
            LDFLAGS += -L../KMATH/fujitsu/lib
        else
            LDFLAGS += -L../KMATH/ntl-5.5.2/src 
        endif
        LDFLAGS += -lkm_random -lntl
    endif
endif

ifneq "$(PBVR_MAKE_CLIENT)" "1"
    NO_CLIENT=1
endif

ifndef MPICXX
    CXXFLAGS += -DCPU_VER
endif

#CXXFLAGS += -DDUPLICATION -DCPU_SAMPLING_TIME -DJPV_CLIENT 
CXXFLAGS += -DCPU_SAMPLING_TIME -DJPV_CLIENT \
            -I${KVS_SOURCE_DIR} \
            -I${KVS_SOURCE_DIR}/Core \
            -I${COMMON_DIR} -I${FUNC_DIR} \
            -I${PBVRFILTER_DIR}/include \
            -I./
            
LDFLAGS  += -L${COMMON_DIR} -lpbvrCommon -L${FUNC_DIR} -lpbvrFunc \
            -L${KVS_SOURCE_DIR}/Core/Release -lkvsCore \
            -L${PBVRFILTER_DIR}/lib -lpbvrFilter 

ifeq "$(PBVR_SUPPORT_VTK)" "1"
    include ${PBVRFILTER_DIR}/Depend.mk
    LDFLAGS += $(LD_VTK_FLAGS)
endif

ifeq "$(NO_CLIENT)" "1"
    CXXFLAGS += -DNO_CLIENT
else
    ifeq "$(shell uname -s)" "Darwin"
        LDFLAGS += -framework glut -framework OpenGL
    else
    ifeq "$(shell uname -s)" "Linux"
        LDFLAGS += -lGLU -lGL -lglut
    endif
    endif
endif

SERVER_SRCS = CellByCellMetropolisSampling.cpp \
              CellByCellRejectionSampling.cpp \
              CellByCellUniformSampling.cpp \
              CropRegion.cpp \
              DataArrayTag.cpp \
              ExtendedTransferFunction.cpp \
              FilterInformation.cpp \
              FrequencyTable.cpp \
              GeometryObjectBase.cpp \
              GlobalCore.cpp \
              AggregateTypeSubvolume.cpp \
              StepAggregateTypeSubvolume.cpp \
              HexahedralCell.cpp \
              IDManager.cpp \
              JobCollector.cpp \
              JobDispatcher.cpp \
              KVSMLObjectPointWriter.cpp \
              SPLITTypeSubvolume.cpp \
              main.cpp \
              MapperBase.cpp \
              ObjectBase.cpp \
              ObjectManager.cpp \
              ParameterFile.cpp \
              PointObject.cpp \
              PointObjectGenerator.cpp \
              PrismaticCell.cpp \
              PyramidalCell.cpp \
              QuadraticHexahedralCell.cpp \
              QuadraticSquareCell.cpp \
              QuadraticTetrahedralCell.cpp \
              QuadraticTriangleCell.cpp \
              RangeEstimater.cpp \
              SquareCell.cpp \
              TetrahedralCell.cpp \
              StructuredVolumeObject.cpp \
              TransferFunction.cpp \
              TransferFunctionSynthesizer.cpp \
              TransferFunctionSynthesizerCreator.cpp \
              TriangleCell.cpp \
              UnstructuredVolumeImporter.cpp \
              UnstructuredVolumeObject.cpp \
              VolumeObjectBase.cpp \
              PbvrFilterInformation.cpp \
              PbvrJobDispatcher.cpp \
              PbvrObjectUnstructuredVolume.cpp \
              Server.cpp \
              PbvrPointObjectGenerator.cpp

SERVER_HEADERS = Argument.h \
                 CellBase.h \
                 CellByCellMetropolisSampling.h \
                 CellByCellParticleGenerator.h \
                 CellByCellRejectionSampling.h \
                 CellByCellUniformSampling.h \
                 common.h \
                 Compiler.h \
                 CropRegion.h \
                 DataArray.h \
                 DataArrayTag.h \
                 endian2.h \
                 ExtendedTransferFunction.h \
                 FilterInformation.h \
                 FrequencyTable.h \
                 GeometryObjectBase.h \
                 GlobalCore.h \
                 AggregateTypeSubvolume.h \
                 StepAggregateTypeSubvolume.h \
                 HexahedralCell.h \
                 IDManager.h \
                 ImporterBase.h \
                 JobCollector.h \
                 JobDispatcher.h \
                 KVSMLObjectPointWriter.h \
                 SPLITTypeSubvolume.h \
                 Macro.h \
                 MapperBase.h \
                 ObjectBase.h \
                 ObjectManager.h \
                 ParameterFile.h \
                 Platform.h \
                 PointObjectGenerator.h \
                 PointObject.h \
                 PrismaticCell.h \
                 PyramidalCell.h \
                 QuadraticHexahedralCell.h \
                 QuadraticSquareCell.h \
                 QuadraticTetrahedralCell.h \
                 QuadraticTriangleCell.h \
                 RangeEstimater.h \
                 SquareCell.h \
                 StructuredVolumeObject.h \
                 TagTable.h \
                 TetrahedralCell.h \
                 timer_simple.h \
                 TransferFunction.h \
                 TransferFunctionSynthesizerCreator.h \
                 TransferFunctionSynthesizer.h \
                 TriangleCell.h \
                 Types.h \
                 UnstructuredVolumeImporter.h \
                 UnstructuredVolumeObject.h \
                 VolumeObjectBase.h \
                 PbvrFilterInformation.h \
                 PbvrJobDispatcher.h \
                 PbvrObjectUnstructuredVolume.h \
                 Server.h \
                 PbvrPointObjectGenerator.h

SERVER_OBJS = $(SERVER_SRCS:%.cpp=%.o)
FUNC_OBJS   = $(FUNC_SRCS:%.cpp=%.o)

OBJS = $(SERVER_OBJS)

all: $(TARGET) 

$(TARGET): $(SERVER_OBJS)
	$(LD) -o $@ $^ $(LDFLAGS)

.cpp.o:
	$(CXX) $(CXXFLAGS) -o $@ -c $<

$(SERVER_OBJS) : $(SERVER_HEADERS)

clean:
	@rm -f *.o

all_clean:
	@rm -f *.o $(TARGET)


