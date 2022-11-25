TARGET   = pbvrFunc
TEMPLATE = lib
CONFIG  += static
QT      += core

include( ../SETTINGS.pri)
message (Building PBVR:$${TARGET} -  Mode:$${PBVR_MODE}  - Platform: $${PLATFORM}  - ReleaseType: $${RELTYPE})

CONFIG += warn_off
QMAKE_MACOSX_DEPLOYMENT_TARGET = 10.15


HEADERS += \
    function.h   \
    function_op.h  \
    function_parser.h

contains(DEFINES, IS_MODE) {
HEADERS += \
    ExpressionConverter.h \
    ExpressionTokenizer.h \
    ReversePolishNotation.h \
    Token.h
}


SOURCES += \
    function.cpp \
    function_parser.cpp


contains(DEFINES, IS_MODE) {
SOURCES += \
    main.cpp \
    ExpressionConverter.cpp \
    ExpressionTokenizer.cpp \
    ReversePolishNotation.cpp

}



