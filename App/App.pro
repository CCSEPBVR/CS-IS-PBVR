include(../SETTINGS.pri)
#=============================================================================
#  Configuration valiable.
#=============================================================================
#CONFIG += release warn_off opengl
CONFIG += warn_off opengl
QT += opengl

greaterThan( QT_MAJOR_VERSION, 5 ) {
QT += openglwidgets
}



#=============================================================================
#  Template.
#=============================================================================
TEMPLATE = app



#=============================================================================
#  Target.
#=============================================================================
TARGET = QTPBVR



#=============================================================================
#  Link library.
#=============================================================================
win32 {
LIBS += ws2_32.lib
LIBS += -L../Widgets/release -lWidgets
LIBS += -L../FunctionParser/release -lpbvrFunc
LIBS += -L../Common/release -lCommon
LIBS += -L../ExtendedKVS/release -lExtendedKVS
LIBS += -L../ExtendedQT/release -lExtendedQT
}

macx {
LIBS += -L../Widgets -lWidgets
LIBS += -L../FunctionParser -lpbvrFunc
LIBS += -L../Common -lCommon
LIBS += -L../ExtendedKVS -lExtendedKVS
LIBS += -L../ExtendedQT -lExtendedQT
}

unix:!macx {
LIBS += -L../Widgets -lWidgets
LIBS += -L../FunctionParser -lpbvrFunc
LIBS += -L../Common -lCommon
LIBS += -L../ExtendedKVS -lExtendedKVS
LIBS += -L../ExtendedQT -lExtendedQT
}

#=============================================================================
#  Header.
#=============================================================================
HEADERS += \
    pbvrgui.h



#=============================================================================
#  Source.
#=============================================================================
SOURCES += \
    main.cpp\
    pbvrgui.cpp



#=============================================================================
#  Forms.
#=============================================================================
FORMS += \
    pbvrgui.ui



#=============================================================================
#  Resource.
#=============================================================================
RESOURCES += \



#=============================================================================
#  Pre Targetdeps
#=============================================================================
win32 {
PRE_TARGETDEPS += ../Widgets/release/Widgets.lib
PRE_TARGETDEPS += ../FunctionParser/release/pbvrFunc.lib
PRE_TARGETDEPS += ../Common/release/Common.lib
PRE_TARGETDEPS += ../ExtendedKVS/release/ExtendedKVS.lib
PRE_TARGETDEPS += ../ExtendedQT/release/ExtendedQT.lib
}

macx {
PRE_TARGETDEPS += ../Widgets/libWidgets.a
PRE_TARGETDEPS += ../FunctionParser/libpbvrFunc.a
PRE_TARGETDEPS += ../Common/libCommon.a
PRE_TARGETDEPS += ../ExtendedKVS/libExtendedKVS.a
PRE_TARGETDEPS += ../ExtendedQT/libExtendedQT.a
}

unix:!macx {
PRE_TARGETDEPS += ../Widgets/libWidgets.a
PRE_TARGETDEPS += ../FunctionParser/libpbvrFunc.a
PRE_TARGETDEPS += ../Common/libCommon.a
PRE_TARGETDEPS += ../ExtendedKVS/libExtendedKVS.a
PRE_TARGETDEPS += ../ExtendedQT/libExtendedQT.a
LIBS += $$KVS_DIR/lib/libkvsSupportQt.a
LIBS += $$KVS_DIR/lib/libkvsCore.a
LIBS += -lGLU
}
