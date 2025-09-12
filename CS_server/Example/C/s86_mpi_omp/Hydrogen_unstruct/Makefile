PBVR_DIR = ../../../..

include ${PBVR_DIR}/pbvr.conf
include ${PBVR_DIR}/arch/${PBVR_MACHINE}

FUNC_DIR        = ${PBVR_DIR}/FunctionParser
INSITU_DIR      = ${PBVR_DIR}/InSituLib/unstruct
COMMON_DIR      = ${PBVR_DIR}/Common
#IMPORT_DIR      = ${INSITU_DIR}/Importer/Importer
VISMODULE_SOURCE_DIR        = ${PBVR_DIR}/VisModule

TARGET  = run

#VPATH=${TRANSFER_FUNCTION_DIR}:${IMPORT_DIR}

CXX = ${MPICXX}
CXXFLAGS += ${MPICXXFLAGS}

CXXFLAGS += -I${INSITU_DIR} \
            -I${FUNC_DIR} \
            -I${COMMON_DIR} \
            -I${VISMODULE_SOURCE_DIR} \
            -I./

LD = ${MPILD}
LDFLAGS += ${MPILDFLAGS}

LDFLAGS  += -L${INSITU_DIR} -lInSituPBVR \
            -L${FUNC_DIR} -lpbvrFunc \
            -L${COMMON_DIR} -lpbvrCommon \
            -L${VISMODULE_SOURCE_DIR}/Release -lvisModule

#LDFLAGS += -DVTK
#LDFLAGS += -I/home/center/viztools/VTK-9.2.2/include/vtk-9.2 -I./VTK -I./Importer -I./Filter
#LDFLAGS += -L/home/center/viztools/VTK-9.2.2/lib64 -lvtkFiltersExtraction-9.2 -lvtkIOXML-9.2 -lvtkIOXMLParser-9.2 -lvtkexpat-9.2 -lvtkParallelCore-9.2 -lvtkIOLegacy-9.2 -lvtkIOCore-9.2 \
#    -lvtkdoubleconversion-9.2 -lvtklz4-9.2 -lvtklzma-9.2 -lvtkFiltersStatistics-9.2 -lvtkFiltersCore-9.2 -lvtkCommonExecutionModel-9.2  \
#    -lvtkzlib-9.2 -lvtkCommonColor-9.2 -lvtkfmt-9.2 -lvtkCommonDataModel-9.2 -lvtkpugixml-9.2 -lvtkCommonSystem-9.2 -lvtkCommonMisc-9.2 -lvtkCommonTransforms-9.2 \
#    -lvtkCommonMath-9.2 -lm -lvtkCommonCore-9.2 -lvtkloguru-9.2 -lvtksys-9.2


#nvcc -O3 -DUSE_NVCC -std=c++11 -restrict -Xcompiler -fopenmp  -lm -lstdc++ -Xcompiler -fopenmp -L/opt/mvapich2/gdr/2.3a/mcast/no-openacc/cuda8.0/mofed4.2/mpirun/gnu4.8.5/lib64  -lmpi -lmpicxx  -L/usr/local/cuda/lib64  -lcudart -rdc=true -Xnvlink -w --gpu-architecture=sm_60 -L../../../InSituLib_LBM -lInSituPBVR -L../../../FunctionParser -lpbvrFunc -L../../../KVS/Source/Core/Release -lkvsCore

TEST_SRCS = \
main.cpp \
Hydrogen.cpp

TEST_HEADERS = \
Hydrogen.h

TEST_OBJS = $(TEST_SRCS:%.cpp=%.o)

OBJS = $(TEST_OBJS)

all: $(TARGET) 

$(TARGET): $(TEST_OBJS)
	$(LD) -o $@ $^ $(LDFLAGS)

.cpp.o:
	$(CXX) $(CXXFLAGS) -o $@ -c $<

$(TEST_OBJS) : $(TEST_HEADERS)

clean:
	@rm -f *.o

all_clean:
	@rm -f *.o $(TARGET)


