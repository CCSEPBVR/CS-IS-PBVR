TARGET   = Common
TEMPLATE = lib
CONFIG  += static
QT      += core

include( ../SETTINGS.pri)
message (Building PBVR:$${TARGET} -  Mode:$${PBVR_MODE}  - Platform: $${PLATFORM}  - ReleaseType: $${RELTYPE})

CONFIG += warn_off
QMAKE_MACOSX_DEPLOYMENT_TARGET = 10.14

DEFINES += JPV_CLIENT


HEADERS += \
    $${PBVR_MODE}/ExtendedTransferFunctionParameter.h   \
    $${PBVR_MODE}/ParticleTransferClient.h   \
    $${PBVR_MODE}/ParticleTransferProtocol.h   \
#    $${PBVR_MODE}/ParticleTransferServer.h   \
    $${PBVR_MODE}/Types.h   \
    $${PBVR_MODE}/VariableRange.h \
    $${PBVR_MODE}/serializer.h \
    CS/VRDummyCamera.h


SOURCES += \
    $${PBVR_MODE}/ExtendedTransferFunctionParameter.cpp   \
    $${PBVR_MODE}/ParticleTransferClient.cpp   \
    $${PBVR_MODE}/ParticleTransferProtocol.cpp   \
#    $${PBVR_MODE}/ParticleTransferServer.cpp   \
    $${PBVR_MODE}/VariableRange.cpp \
    $${PBVR_MODE}/serializer.cpp \
    CS/VRDummyCamera.cpp



