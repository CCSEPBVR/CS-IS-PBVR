TARGET   = Common
TEMPLATE = lib
CONFIG  += static
QT      += core

include( ../SETTINGS.pri)
message (Building PBVR:$${TARGET} -  Mode:$${PBVR_MODE}  - Platform: $${PLATFORM}  - ReleaseType: $${RELTYPE})

CONFIG += warn_off
QMAKE_MACOSX_DEPLOYMENT_TARGET = 10.15

DEFINES += JPV_CLIENT


HEADERS += \
    $${COMM_MODE_STR}/ExtendedTransferFunctionParameter.h   \
    $${COMM_MODE_STR}/ParticleTransferClient.h   \
    $${COMM_MODE_STR}/ParticleTransferProtocol.h   \
#    $${COMM_MODE_STR}/ParticleTransferServer.h   \
    $${COMM_MODE_STR}/Types.h   \
    $${COMM_MODE_STR}/VariableRange.h \
    $${COMM_MODE_STR}/serializer.h


SOURCES += \
    $${COMM_MODE_STR}/ExtendedTransferFunctionParameter.cpp   \
    $${COMM_MODE_STR}/ParticleTransferClient.cpp   \
    $${COMM_MODE_STR}/ParticleTransferProtocol.cpp   \
#    $${COMM_MODE_STR}/ParticleTransferServer.cpp   \
    $${COMM_MODE_STR}/VariableRange.cpp \
    $${COMM_MODE_STR}/serializer.cpp
