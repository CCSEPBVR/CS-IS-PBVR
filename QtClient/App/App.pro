TARGET = pbvr_client
TEMPLATE = app
#CONFIG += static
QT += core gui opengl widgets openglwidgets
CONFIG += console
#PRECOMPILED_HEADER=stable.h

CONFIG += warn_off
QMAKE_MACOSX_DEPLOYMENT_TARGET = 10.14

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
win32{
LIBS += -L$$(GLEW_DIR)/lib -lglew32
LIBS += -L$$(OCULUS_LIB_DIR) -lLibOVR
LIBS += -L$$(IMGUI_DIR) -llibimgui -llibimgui_impl_opengl3
#-llibimgui_impl_win32
LIBS += -L$$(CGFORMAT_EXT4KVS_SHADER_DIR) -lLibCGFormatExt4KVS
LIBS += -L$$(FBX_SDK_LIB_DIR) -llibfbxsdk-mt -llibxml2-mt -lzlib-mt -ladvapi32
LIBS += -L$$(ASSIMP_LIB_DIR) -lassimp-vc142-mt
}
else {
LIBS += -L$$(GLEW_DIR)/lib
}
unix:macx:LIBS +=  -lglew
unix:!macx:LIBS += -lGLEW -lGLU
#QT += opengl
#QT += core gui opengl widgets
#QMAKE_CXXFLAGS += -DQT_NO_OPENGL_ES_2
#QMAKE_CXXFLAGS += -DQT_OPENGL_ES
#QMAKE_CXXFLAGS += -DQT_NO_OPENGL_ES
#LIBS += -lOpenGL32
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
