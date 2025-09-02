PBVR_DIR = ../../..

include ${PBVR_DIR}/pbvr.conf
include ${PBVR_DIR}/arch/${PBVR_MACHINE}

KVS_SOURCE_DIR  = ${PBVR_DIR}/KVS/Source
FUNC_DIR        = ${PBVR_DIR}/FunctionParser
INSITU_DIR      = ${PBVR_DIR}/InSituLib/struct

TARGET  = run

CXX = ${MPICXX}
CXXFLAGS = ${MPICXXFLAGS}
#CXXFLAGS = -lstdc++ -O3 -fopenmp -march=native

CXXFLAGS += -I${INSITU_DIR} \
            -I${FUNC_DIR} \
            -I${KVS_SOURCE_DIR} \
            -I./

FXX = mpif90
FXXFLAGS = ${MPICXXFLAGS}

FXXFLAGS += -I${INSITU_DIR} \
            -I${FUNC_DIR} \
            -I${KVS_SOURCE_DIR} \
            -I./

#CXXFLAGS += -I${INSITU_DIR} \

LD = ${FXX}
#LD = ${MPILD}
LDFLAGS = ${MPILDFLAGS}
#LDFLAGS = -lstdc++ -fopenmp -static-libgcc -static-libstdc++

#LDFLAGS  += -lmpi -lmpicxx
LDFLAGS  += -lmpi -lmpi++ -lstdc++
LDFLAGS  += -L${INSITU_DIR} -lInSituPBVR \
            -L${FUNC_DIR} -lpbvrFunc \
            -L${KVS_SOURCE_DIR}/Core/Release -lkvsCore

TEST_OBJS_F = Hydrogen_F.o main_F.o
TEST_OBJS_C = wrapper_f.o

TEST_OBJS = $(TEST_OBJS_C) $(TEST_OBJS_F)

all: $(TARGET) 

$(TARGET): $(TEST_OBJS)
	$(LD) -o $@ $^ $(LDFLAGS)

%.o : %.cpp
	$(CXX) $(CXXFLAGS) -o $@ -c $<

%.o : %.f90
	$(FXX) $(FXXFLAGS) -o $@ -c $<

#$(TEST_OBJS) : $(TEST_HEADERS)

clean:
	@rm -f *.o

all_clean:
	@rm -f *.o $(TARGET)


