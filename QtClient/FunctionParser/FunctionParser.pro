TARGET   = pbvrFunc
TEMPLATE = lib
CONFIG  += static
QT      += core

include( ../SETTINGS.pri)
message (Building PBVR:$${TARGET} -  Mode:$${PBVR_MODE}  - Platform: $${PLATFORM}  - ReleaseType: $${RELTYPE})

CONFIG += warn_off
QMAKE_MACOSX_DEPLOYMENT_TARGET = 10.14

HEADERS += \
    function.h   \
    function_op.h  \
    function_parser.h

SOURCES += \
    function.cpp \
    function_parser.cpp




