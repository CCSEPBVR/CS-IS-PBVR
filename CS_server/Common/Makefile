include ../pbvr.conf
include ../arch/${PBVR_MACHINE}
TARGET=libpbvrCommon.a
FUNC_DIR=../FunctionParser
KVS_SOURCE_DIR=../KVS/Source
CXXFLAGS+=-DDUPLICATION -DCPU_SAMPLING_TIME -DJPV_CLIENT -I${KVS_SOURCE_DIR} -I${FUNC_DIR}

ifneq "$(PBVR_MAKE_CLIENT)" "1"
    NO_CLIENT=1
endif

ifeq "$(NO_CLIENT)" "1"
    CXXFLAGS+=-DNO_CLIENT
endif

COMMON_SRCS = ParticleTransferClient.cpp \
              ParticleTransferProtocol.cpp \
              ParticleTransferServer.cpp \
              Serializer.cpp \
              ExtendedTransferFunctionParameter.cpp \
              VariableRange.cpp \
			  Timer.cpp


COMMON_HEADERS = ParticleTransferClient.h \
                 ParticleTransferProtocol.h \
                 ParticleTransferServer.h \
                 Serializer.h \
                 ExtendedTransferFunctionParameter.h \
                 VariableRange.h \
				 Timer.h

COMMON_OBJS   = $(COMMON_SRCS:%.cpp=%.o)

all: $(TARGET) 

$(TARGET): $(COMMON_OBJS)
	$(AR) $(ARFLAGS) $(TARGET) $(COMMON_OBJS)

.cpp.o:
	$(CXX) $(CXXFLAGS) -o $@ -c $<

$(COMMON_OBJS) : $(COMMON_HEADERS)

clean:
	@rm -f *.o

all_clean:
	@rm -f *.o $(TARGET)
