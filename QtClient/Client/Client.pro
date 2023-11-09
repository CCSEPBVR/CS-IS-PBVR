TARGET   = Client
TEMPLATE = lib
CONFIG += static
QT += core gui opengl widgets openglwidgets opengl

#PRECOMPILED_HEADER=stable.h

CONFIG += warn_off
QMAKE_MACOSX_DEPLOYMENT_TARGET = 10.15

include( ../SETTINGS.pri)
message (Building PBVR:$${TARGET}  Mode:$${PBVR_MODE}  Platform: $${PLATFORM} Release: $${RELTYPE})

HEADERS += \
    Argument.h \
    ColorMapLibrary.h \
    Command.h \
    ComThread.h \
    LogManager.h \
    endian2.h \
    FrequencyTable.h \
    KeyFrameAnimation.h \
    KeyPressEvent.h \
    KVSMLObjectPointWriter.h \
    ParticleMerger.h \
    timer_simple.h \
    TimerEvent.h \
    VizParameterFile.h \
    v3defines.h \
    stable.h \
    ParamExTransFunc.h \
    VisualizationParameter.h \
    ReceivedMessage.h \
    ParameterFile.h \
    ParticleMergeParameter.h \
    PBRProxy.h


SOURCES += \
    ColorMapLibrary.cpp \
    ComThread.cpp \
    KeyFrameAnimation.cpp \
    KVSMLObjectPointWriter.cpp \
    LogManager.cpp \
    ParticleMerger.cpp \
    TimerEvent.cpp \
    VizParameterFile.cpp \
    ParameterFile.cpp \
    PBRProxy.cpp


HEADERS += ExtendedTransferFunctionMessage_$${COMM_MODE_STR}.h
SOURCES += ExtendedTransferFunctionMessage_$${COMM_MODE_STR}.cpp

HEADERS += ParticleServer.h
SOURCES += ParticleServer_$${COMM_MODE_STR}.cpp

SOURCES += Command_$${COMM_MODE_STR}.cpp


