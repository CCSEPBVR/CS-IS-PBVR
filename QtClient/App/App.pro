TARGET = pbvr_client
TEMPLATE = app
#CONFIG += static
QT       += core gui opengl widgets
CONFIG += console
#PRECOMPILED_HEADER=stable.h

CONFIG += warn_off
QMAKE_MACOSX_DEPLOYMENT_TARGET = 10.15

include(../SETTINGS.pri)
message (Building PBVR:$${TARGET} - RenderMode:$${RENDER_MODE} Mode:$${PBVR_MODE}  - Platform: $${PLATFORM}  - ReleaseType: $${RELTYPE})

#LINK EXTERNAL LIBS
#LIBS += kvsCore.lib
#LIBS += kvsSupportGLEW.lib

LIBS += -L../Panels -lPanels
LIBS += -L../QGlue  -lQGlue
LIBS += -L../Client -lClient
LIBS += -L../FunctionParser -lpbvrFunc
LIBS += -L../Common -lCommon
#LIBS += -L$${KVS_DIR} -lKVS
LIBS += -L$$(KVS_DIR)/lib -lkvsCore

!isEmpty(KVS_GLEW_DIR){
    win32{
        LIBS += -L$$(KVS_GLEW_DIR)/lib -lglew32
    }
    else {
        LIBS += -L$$(KVS_GLEW_DIR)/lib
    }
    unix:macx:LIBS +=  -lglew
    unix:!macx:LIBS += -lGLEW -lGLU
}

win32{
LIBS += glu32.lib
LIBS += opengl32.lib
LIBS += ws2_32.lib
}
#LOCAL INCLUDE PATHS FOR CONVIENIENCE
INCLUDEPATH += $$PWD/..

HEADERS += \
    pbvrgui.h \
    helpdialog.h \
    stable.h

SOURCES += \
    pbvrgui.cpp \
    helpdialog.cpp \
    main.cpp

FORMS += \
    pbvrgui.ui \
    helpdialog.ui

RESOURCES += \
    resources.qrc

PRE_TARGETDEPS += ../Client/$${LIB_PREFIX}Client$${LIB_SUFFIX}
PRE_TARGETDEPS += ../QGlue/$${LIB_PREFIX}QGlue$${LIB_SUFFIX}
PRE_TARGETDEPS += ../Panels/$${LIB_PREFIX}Panels$${LIB_SUFFIX}
PRE_TARGETDEPS += ../Common/$${LIB_PREFIX}Common$${LIB_SUFFIX}
PRE_TARGETDEPS += ../FunctionParser/$${LIB_PREFIX}pbvrFunc$${LIB_SUFFIX}
