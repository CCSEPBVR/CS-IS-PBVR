include ../pbvr.conf
include ../arch/${PBVR_MACHINE}

TARGET        = pbvr_server
FUNC_DIR      = ../FunctionParser
COMMON_DIR    = ../Common/
KVS_SOURCE_DIR= ../KVS/Source

ifdef MPICXX
    CXX     = ${MPICXX}
endif

ifdef MPICXXFLAGS
    CXXFLAGS = ${MPICXXFLAGS}
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

ifndef MPICXX
    CXXFLAGS += -DCPU_VER
endif

#CXXFLAGS += -w  -DDUPLICATION -DCPU_SAMPLING_TIME -DJPV_CLIENT -I${KVS_SOURCE_DIR} -I${COMMON_DIR} -I${FUNC_DIR} -I./
#CXXFLAGS += -w -msse2 -DDUPLICATION -DCPU_SAMPLING_TIME -DJPV_CLIENT -I${KVS_SOURCE_DIR} -I${COMMON_DIR} -I${FUNC_DIR} -I./
#CXXFLAGS += -w -qopt-report=5 -DSFMT_MEXP=607 -DCPU_SAMPLING_TIME -DJPV_CLIENT -I${KVS_SOURCE_DIR} -I${COMMON_DIR} -I${FUNC_DIR} -I./
CXXFLAGS += -w -DCPU_SAMPLING_TIME -DJPV_CLIENT -I${KVS_SOURCE_DIR} -I${COMMON_DIR} -I${FUNC_DIR} -I./
LDFLAGS  += -L${COMMON_DIR} -lpbvrCommon -L${FUNC_DIR} -lpbvrFunc -L${KVS_SOURCE_DIR}/Core/Release -lkvsCore

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
              QuadraticTetrahedralCell.cpp \
              RangeEstimater.cpp \
              StructuredVolumeImporter.cpp \
              StructuredVolumeObject.cpp \
              TetrahedralCell.cpp \
			  timer.cpp \
              StructuredVolumeObject.cpp \
		      SFMT/SFMT.cpp\
		      TransferFunction.cpp \
              TransferFunctionSynthesizer.cpp \
              TransferFunctionSynthesizerCreator.cpp \
              UnstructuredVolumeImporter.cpp \
              UnstructuredVolumeObject.cpp \
              VolumeObjectBase.cpp 
#              TriangleCell.cpp \
#              SquareCell.cpp \
#              QuadraticSquareCell.cpp \
#              QuadraticTriangleCell.cpp \


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
				 FileChecker.h \
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
                 QuadraticTetrahedralCell.h \
                 RangeEstimater.h \
                 StructuredVolumeObject.h \
                 StructuredVolumeImporter.h \
                 TagTable.h \
                 TetrahedralCell.h \
                 timer_simple.h \
				 timer.h \
		         SFMT/SFMT.h\
                 TransferFunction.h \
                 TransferFunctionSynthesizerCreator.h \
                 TransferFunctionSynthesizer.h \
                 Types.h \
                 UnstructuredVolumeImporter.h \
                 UnstructuredVolumeObject.h \
                 VolumeObjectBase.h 
#                 SquareCell.h \
#                 TriangleCell.h \
#                 QuadraticTriangleCell.h \
#                 QuadraticSquareCell.h 

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
	@rm -f *.o *optrpt SFMT/*o

all_clean:
	@rm -f *.o $(TARGET)


