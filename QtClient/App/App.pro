TARGET = pbvr_client
TEMPLATE = app
#CONFIG += static
QT += core gui opengl widgets openglwidgets
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

contains(DEFINES, ENABLE_FBXSDK)|contains(DEFINES, ENABLE_ASSIMP) {
}
LIBS += -L$$(CGFORMAT_EXT4KVS_SHADER_DIR) -lLibCGFormatExt4KVS
contains(DEFINES, ENABLE_FBXSDK) {
    win32 {
        LIBS += -L$$(FBX_SDK_LIB_DIR) -llibfbxsdk-mt -llibxml2-mt -lzlib-mt -ladvapi32
    }
    else {
        LIBS += -L$$(FBX_SDK_LIB_DIR) -lfbxsdk
    }
    unix:!macx:LIBS += -lxml2

}
contains(DEFINES, ENABLE_ASSIMP) {
    win32 {
        LIBS += -L$$(ASSIMP_LIB_DIR) -lassimp-vc142-mt
    }
    else {
        LIBS += -L$$(ASSIMP_LIB_DIR) -lassimp -lIrrXML -lzlibstatic
    }
}

contains(DEFINES, ENABLE_LIBLAS) {
    win32 {
        LIBS += -L$$(BOOST_ROOT)/lib -lboost_thread-vc142-mt-x64-1_83
        LIBS += -L$$(LIBLAS_LIB_DIR) -lliblas
    }
    else {
        LIBS += -L$$(LIBLAS_LIB_DIR) -llas
    }
}

## for VR/MR mode (windows only)
win32{
    contains(DEFINES, VR_MODE)|contains(DEFINES, MR_MODE) {
        LIBS += -L$$(OCULUS_LIB_DIR) -lLibOVR
        LIBS += -L$$(IMGUI_DIR) -llibimgui -llibimgui_impl_opengl3
        #-llibimgui_impl_win32

        contains(DEFINES, MR_MODE) {
            LIBS += -L$$(CUDA_PATH)/lib/x64 -lcuda -lcudart_static -lcublas
            LIBS += -L$$(ZED_SDK_ROOT_DIR)/lib -lsl_zed64
        }
    }
}

## for GLEW (Windows & linux)
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
