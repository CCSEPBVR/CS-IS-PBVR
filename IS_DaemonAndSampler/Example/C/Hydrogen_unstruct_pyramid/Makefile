PBVR_DIR = ../../..

include ${PBVR_DIR}/pbvr.conf
include ${PBVR_DIR}/arch/${PBVR_MACHINE}

KVS_SOURCE_DIR  = ${PBVR_DIR}/KVS/Source
FUNC_DIR        = ${PBVR_DIR}/FunctionParser
INSITU_DIR      = ${PBVR_DIR}/InSituLib/unstruct

TARGET  = run

CXX = ${MPICXX}
CXXFLAGS = ${MPICXXFLAGS}

CXXFLAGS += -w -I${INSITU_DIR} \
            -I${FUNC_DIR} \
            -I${KVS_SOURCE_DIR} \
            -I./
#CXXFLAGS += -DVTK 

LD = ${MPILD}
LDFLAGS = ${MPILDFLAGS}

#LDFLAGS  += -lmpi -lmpicxx
LDFLAGS  += -L${INSITU_DIR} -lInSituPBVR \
            -L${FUNC_DIR} -lpbvrFunc \
            -L${KVS_SOURCE_DIR}/Core/Release -lkvsCore


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


