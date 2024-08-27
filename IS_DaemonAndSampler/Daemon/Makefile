include ../pbvr.conf
include ../arch/${PBVR_MACHINE}

TARGET        = pbvr_daemon
FUNC_DIR      = ../FunctionParser
COMMON_DIR    = ../Common/
KVS_SOURCE_DIR= ../KVS/Source

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

CXXFLAGS += -DCPU_VER
#CXXFLAGS += -DDUPLICATION -DCPU_SAMPLING_TIME -DJPV_CLIENT -I${KVS_SOURCE_DIR} -I${COMMON_DIR} -I${FUNC_DIR} -I./
CXXFLAGS += -DCPU_SAMPLING_TIME -DJPV_CLIENT -I${KVS_SOURCE_DIR} -I${COMMON_DIR} -I${FUNC_DIR} -I./
LDFLAGS  += -L${COMMON_DIR} -lpbvrCommon -L${FUNC_DIR} -lpbvrFunc -L${KVS_SOURCE_DIR}/Core/Release -lkvsCore

DAEMON_SRCS = CropRegion.cpp \
              DataArrayTag.cpp \
              FilterInfo.cpp \
              FrequencyTable.cpp \
              GeometryObjectBase.cpp \
              GlobalCore.cpp \
              IDManager.cpp \
              JobCollector.cpp \
              JobDispatcher.cpp \
              KVSMLObjectPointWriter.cpp \
              main.cpp \
              ObjectBase.cpp \
              ObjectManager.cpp \
              ParamInfo.cpp \
              ParameterFileReader.cpp \
              ParameterFileWriter.cpp \
              ParticleFile.cpp \
              ParticleMonitor.cpp \
              PointImporter.cpp \
              ParticleStatusFile.cpp \
              ParticleHistoryFile.cpp \
              PointObject.cpp \
              TransferFunction.cpp \
              Timer.cpp \
              VolumeObjectBase.cpp \
	      ExtendedTransferFunctionMessage.cpp


DAEMON_HEADERS = Argument.h \
                 common.h \
                 Compiler.h \
                 CropRegion.h \
                 DataArray.h \
                 DataArrayTag.h \
                 endian2.h \
                 FilterInfo.h \
                 FrequencyTable.h \
                 GeometryObjectBase.h \
                 GlobalCore.h \
                 IDManager.h \
                 ImporterBase.h \
                 JobCollector.h \
                 JobDispatcher.h \
                 KVSMLObjectPointWriter.h \
                 Macro.h \
                 NameListFile.h \
                 ObjectBase.h \
                 ObjectManager.h \
                 ParamInfo.h \
                 ParameterFileReader.h \
                 ParameterFileWriter.h \
                 ParticleFile.h \
                 ParticleMonitor.h \
                 ParticleStatusFile.h \
                 ParticleHistoryFile.h \
                 Platform.h \
                 PointImporter.h \
                 PointObject.h \
                 TagTable.h \
                 timer_simple.h \
                 Timer.h \
                 TransferFunction.h \
                 Types.h \
                 VolumeObjectBase.h \
		 PBVRParam.h \
	         ExtendedTransferFunctionMessage.cpp \
                 CropParam.h \
                 ParticleMergeParam.h

DAEMON_OBJS = $(DAEMON_SRCS:%.cpp=%.o)
FUNC_OBJS   = $(FUNC_SRCS:%.cpp=%.o)

OBJS = $(DAEMON_OBJS)

all: $(TARGET) 

$(TARGET): $(DAEMON_OBJS)
	$(LD) -o $@ $^ $(LDFLAGS)

.cpp.o:
	$(CXX) $(CXXFLAGS) -o $@ -c $<

$(DAEMON_OBJS) : $(DAEMON_HEADERS)

clean:
	@rm -f *.o

all_clean:
	@rm -f *.o $(TARGET)


