PBVR_DIR = ../../../..

include ${PBVR_DIR}/pbvr.conf
include ${PBVR_DIR}/arch/${PBVR_MACHINE}

INSITU_DIR           = ${PBVR_DIR}/InSituLib/struct
INSITU_COMMON_DIR    = ${PBVR_DIR}/InSituLib/shared
VISMODULE_SOURCE_DIR = ${PBVR_DIR}/VisModule
FUNC_DIR             = ${PBVR_DIR}/FunctionParser
COMMON_DIR           = ${PBVR_DIR}/Common

TARGET  = run

ifdef MPICXX
    CXX      = ${MPICXX}
endif

ifdef MPICXXFLAGS
    CXXFLAGS = ${MPICXXFLAGS}
else
	CXXFLAGS += -DCPU_VER
endif

CXXFLAGS += -I./ \
            -I${INSITU_DIR} \
            -I${INSITU_COMMON_DIR} \
            -I${VISMODULE_SOURCE_DIR} \
            -I${FUNC_DIR} \
            -I${COMMON_DIR}

ifdef MPICXX
    LD = ${MPILD}
endif

ifdef MPILDFLAGS
    LDFLAGS += ${MPILDFLAGS}
endif

LDFLAGS  += -L${INSITU_DIR} -lInSituPBVR \
            -L${VISMODULE_SOURCE_DIR}/Release -lvisModule \
            -L${FUNC_DIR} -lpbvrFunc \
            -L${COMMON_DIR} -lpbvrCommon

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
