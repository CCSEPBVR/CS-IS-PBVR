include(../SETTINGS.pri)
include(TestsConfig.pri)

TEMPLATE = app
CONFIG += console testcase
QT += testlib

TARGET = pbvr_client_qtest
CLIENT_DIR = $$clean_path($$OUT_PWD/..)

INCLUDEPATH += $$PWD
INCLUDEPATH += $$PWD/../App

HEADERS += \
    ../App/MainWindow.h \
    TestAppContext.h \
    TestOutputPaths.h

SOURCES += \
    TestMain.cpp \
    TestOutputPaths.cpp \
    ../App/MainWindow.cpp

DEFINES += PBVR_TEST_NO_MAIN

equals(TEST_ENABLE_MENUBAR, 1) {
    DEFINES += PBVR_ENABLE_TEST_MENUBAR
    HEADERS += MenuBarTest.h
    SOURCES += MenuBarTest.cpp
}

equals(TEST_ENABLE_SCREEN, 1) {
    DEFINES += PBVR_ENABLE_TEST_SCREEN
    HEADERS += ScreenTest.h
    SOURCES += ScreenTest.cpp
}

equals(TEST_ENABLE_PLAYBACKCONTROLTOOLBAR, 1) {
    DEFINES += PBVR_ENABLE_TEST_PLAYBACKCONTROLTOOLBAR
    HEADERS += PlayBackControlToolBarTest.h
    SOURCES += PlayBackControlToolBarTest.cpp
}

equals(TEST_ENABLE_TIMESTEPCONTROLTOOLBAR, 1) {
    DEFINES += PBVR_ENABLE_TEST_TIMESTEPCONTROLTOOLBAR
    HEADERS += TimeStepControlToolBarTest.h
    SOURCES += TimeStepControlToolBarTest.cpp
}

equals(TEST_ENABLE_COLORMAPSELECTORTOOLBAR, 1) {
    DEFINES += PBVR_ENABLE_TEST_COLORMAPSELECTORTOOLBAR
    HEADERS += ColorMapSelectorToolBarTest.h
    SOURCES += ColorMapSelectorToolBarTest.cpp
}

equals(TEST_ENABLE_TOTALPARTICLESTOOLBAR, 1) {
    DEFINES += PBVR_ENABLE_TEST_TOTALPARTICLESTOOLBAR
    HEADERS += TotalParticlesToolBarTest.h
    SOURCES += TotalParticlesToolBarTest.cpp
}

equals(TEST_ENABLE_PREFERENCE, 1) {
    DEFINES += PBVR_ENABLE_TEST_PREFERENCE
    HEADERS += PreferenceTest.h
    SOURCES += PreferenceTest.cpp
}

equals(TEST_ENABLE_COMMUNICATION, 1) {
    DEFINES += PBVR_ENABLE_TEST_COMMUNICATION
    HEADERS += CommunicationTest.h
    SOURCES += CommunicationTest.cpp
}

equals(TEST_ENABLE_COMMUNICATION_USER_INFO, 1) {
    DEFINES += PBVR_ENABLE_TEST_COMMUNICATION_USER_INFO
    HEADERS += CommunicationUserInfoTest.h
    SOURCES += CommunicationUserInfoTest.cpp
}

equals(TEST_ENABLE_COMMUNICATION_SETTING, 1) {
    DEFINES += PBVR_ENABLE_TEST_COMMUNICATION_SETTING
    HEADERS += CommunicationSettingTest.h
    SOURCES += CommunicationSettingTest.cpp
}

equals(TEST_ENABLE_COMMUNICATION_SHARE_VIEW, 1) {
    DEFINES += PBVR_ENABLE_TEST_COMMUNICATION_SHARE_VIEW
    HEADERS += CommunicationShareViewTest.h
    SOURCES += CommunicationShareViewTest.cpp
}

equals(TEST_ENABLE_ANIMATIONCONTROL, 1) {
    DEFINES += PBVR_ENABLE_TEST_ANIMATIONCONTROL
    HEADERS += AnimationControlTest.h
    SOURCES += AnimationControlTest.cpp
}

equals(TEST_ENABLE_GLYPHEDITOR, 1) {
    DEFINES += PBVR_ENABLE_TEST_GLYPHEDITOR
    HEADERS += GlyphEditorTest.h
    SOURCES += GlyphEditorTest.cpp
}

equals(TEST_ENABLE_OBJECTEDITOR, 1) {
    DEFINES += PBVR_ENABLE_TEST_OBJECTEDITOR
    HEADERS += ObjectEditorTest.h
    SOURCES += ObjectEditorTest.cpp
}

equals(TEST_ENABLE_PLOTOVERLINEEDITOR, 1) {
    DEFINES += PBVR_ENABLE_TEST_PLOTOVERLINEEDITOR
    HEADERS += PlotOverLineEditorTest.h
    SOURCES += PlotOverLineEditorTest.cpp
}

equals(TEST_ENABLE_POINTSIZECONTROL, 1) {
    DEFINES += PBVR_ENABLE_TEST_POINTSIZECONTROL
    HEADERS += PointSizeControlTest.h
    SOURCES += PointSizeControlTest.cpp
}

equals(TEST_ENABLE_REPETITIONLEVELCONTROL, 1) {
    DEFINES += PBVR_ENABLE_TEST_REPETITIONLEVELCONTROL
    HEADERS += RepetitionLevelControlTest.h
    SOURCES += RepetitionLevelControlTest.cpp
}

equals(TEST_ENABLE_SHADINGCONTROL, 1) {
    DEFINES += PBVR_ENABLE_TEST_SHADINGCONTROL
    HEADERS += ShadingControlTest.h
    SOURCES += ShadingControlTest.cpp
}

equals(TEST_ENABLE_SERVER, 1) {
    DEFINES += PBVR_ENABLE_TEST_SERVER
    HEADERS += ServerTest.h
    SOURCES += ServerTest.cpp
}

equals(TEST_ENABLE_VOLUMETRANSFORM, 1) {
    DEFINES += PBVR_ENABLE_TEST_VOLUMETRANSFORM
    HEADERS += VolumeTransformTest.h
    SOURCES += VolumeTransformTest.cpp
}

equals(TEST_ENABLE_TRANSFERFUNCTIONEDITOR_IMPORTEXPORT, 1) {
    DEFINES += PBVR_ENABLE_TEST_TRANSFERFUNCTIONEDITOR_IMPORTEXPORT
    HEADERS += TransferFunctionEditorImportExportTest.h
    SOURCES += TransferFunctionEditorImportExportTest.cpp
}

equals(TEST_ENABLE_TRANSFERFUNCTIONEDITOR_CHANGE_TRANSFER_FUNCTION_NUMBER, 1) {
    DEFINES += PBVR_ENABLE_TEST_TRANSFERFUNCTIONEDITOR_CHANGE_TRANSFER_FUNCTION_NUMBER
    HEADERS += TransferFunctionEditorChangeTransferFunctionNumberTest.h
    SOURCES += TransferFunctionEditorChangeTransferFunctionNumberTest.cpp
}

equals(TEST_ENABLE_TRANSFERFUNCTIONEDITOR_COLOR_FUNCTION_SYNTHESIZER, 1) {
    DEFINES += PBVR_ENABLE_TEST_TRANSFERFUNCTIONEDITOR_COLOR_FUNCTION_SYNTHESIZER
    HEADERS += TransferFunctionEditorColorFunctionSynthesizerTest.h
    SOURCES += TransferFunctionEditorColorFunctionSynthesizerTest.cpp
}

equals(TEST_ENABLE_TRANSFERFUNCTIONEDITOR_OPACITY_FUNCTION_SYNTHESIZER, 1) {
    DEFINES += PBVR_ENABLE_TEST_TRANSFERFUNCTIONEDITOR_OPACITY_FUNCTION_SYNTHESIZER
    HEADERS += TransferFunctionEditorOpacityFunctionSynthesizerTest.h
    SOURCES += TransferFunctionEditorOpacityFunctionSynthesizerTest.cpp
}

equals(TEST_ENABLE_TRANSFERFUNCTIONEDITOR_OPACITY_FUNCTION_VARIABLE, 1) {
    DEFINES += PBVR_ENABLE_TEST_TRANSFERFUNCTIONEDITOR_OPACITY_FUNCTION_VARIABLE
    HEADERS += TransferFunctionEditorOpacityFunctionVariableTest.h
    SOURCES += TransferFunctionEditorOpacityFunctionVariableTest.cpp
}

equals(TEST_ENABLE_TRANSFERFUNCTIONEDITOR_COLOR_FUNCTION_VARIABLE, 1) {
    DEFINES += PBVR_ENABLE_TEST_TRANSFERFUNCTIONEDITOR_COLOR_FUNCTION_VARIABLE
    HEADERS += TransferFunctionEditorColorFunctionVariableTest.h
    SOURCES += TransferFunctionEditorColorFunctionVariableTest.cpp
}

equals(TEST_ENABLE_TRANSFERFUNCTIONEDITOR_COLOR_MIN_MAX, 1) {
    DEFINES += PBVR_ENABLE_TEST_TRANSFERFUNCTIONEDITOR_COLOR_MIN_MAX
    HEADERS += TransferFunctionEditorColorMinMaxTest.h
    SOURCES += TransferFunctionEditorColorMinMaxTest.cpp
}

equals(TEST_ENABLE_TRANSFERFUNCTIONEDITOR_OPACITY_MIN_MAX, 1) {
    DEFINES += PBVR_ENABLE_TEST_TRANSFERFUNCTIONEDITOR_OPACITY_MIN_MAX
    HEADERS += TransferFunctionEditorOpacityMinMaxTest.h
    SOURCES += TransferFunctionEditorOpacityMinMaxTest.cpp
}

equals(TEST_ENABLE_TRANSFERFUNCTIONEDITOR_COLOR_MAP_EDIT, 1) {
    DEFINES += PBVR_ENABLE_TEST_TRANSFERFUNCTIONEDITOR_COLOR_MAP_EDIT
    HEADERS += TransferFunctionEditorColorMapEditTest.h
    SOURCES += TransferFunctionEditorColorMapEditTest.cpp
}

equals(TEST_ENABLE_TRANSFERFUNCTIONEDITOR_OPACITY_MAP_EDIT, 1) {
    DEFINES += PBVR_ENABLE_TEST_TRANSFERFUNCTIONEDITOR_OPACITY_MAP_EDIT
    HEADERS += TransferFunctionEditorOpacityMapEditTest.h
    SOURCES += TransferFunctionEditorOpacityMapEditTest.cpp
}

FORMS += \
    ../App/MainWindow.ui

RESOURCES += \
    ../App/Resources.qrc

win32 {
    LIBS += ws2_32.lib
    LIBS += user32.lib
    CONFIG(release, debug|release){
        LIBS += -L$$CLIENT_DIR/Widgets/release -lWidgets
        LIBS += -L$$CLIENT_DIR/FunctionParser/release -lFunctionParser
        LIBS += -L$$CLIENT_DIR/Utils/release -lUtils
        LIBS += -L$$CLIENT_DIR/ExtendedKVS/release -lExtendedKVS
        LIBS += -L$$CLIENT_DIR/ExtendedQT/release -lExtendedQT
    }
    else:CONFIG(debug, debug|release){
        LIBS += -L$$CLIENT_DIR/Widgets/debug -lWidgets
        LIBS += -L$$CLIENT_DIR/FunctionParser/debug -lFunctionParser
        LIBS += -L$$CLIENT_DIR/Utils/debug -lUtils
        LIBS += -L$$CLIENT_DIR/ExtendedKVS/debug -lExtendedKVS
        LIBS += -L$$CLIENT_DIR/ExtendedQT/debug -lExtendedQT
    }

    !isEmpty( PBVR_KVS_INSTALL_DIR ) {
        equals( KVS_SUPPORT_QT, "1" ) {
            LIBS += -L$$PBVR_KVS_INSTALL_DIR/lib -lkvsSupportQt -lkvsCore
        }
        equals( KVS_ENABLE_OPENGL, "1" ) {
            LIBS += -lopengl32
        }
        equals( KVS_ENABLE_GLU, "1" ) {
            LIBS += -lglu32
        }
        equals( KVS_ENABLE_GLEW, "1" ) {
            LIBS += -L$$KVS_GLEW_DIR/lib -lglew32
        }
        equals( KVS_SUPPORT_GLUT, "1" ) {
            LIBS += -L$$KVS_GLUT_DIR/lib -lfreeglut
            LIBS += -L$$PBVR_KVS_INSTALL_DIR/lib -lkvsSupportGLUT
        }
        equals( KVS_SUPPORT_OPENXR, "1" ) {
            LIBS += -L$$PBVR_KVS_INSTALL_DIR/lib -lkvsSupportOpenXR
            LIBS += -L$$KVS_OPENXR_DIR/lib -lopenxr_loader
            LIBS += -lgdi32
        }
    }
    !isEmpty( KVS_ASSIMP_DIR ) {
        LIBS += -L$$KVS_ASSIMP_DIR/lib -lassimp-vc143-mt
    }
}

macx {
    LIBS += -L$$CLIENT_DIR/Widgets -lWidgets
    LIBS += -L$$CLIENT_DIR/FunctionParser -lFunctionParser
    LIBS += -L$$CLIENT_DIR/Utils -lUtils
    LIBS += -L$$CLIENT_DIR/ExtendedKVS -lExtendedKVS
    LIBS += -L$$CLIENT_DIR/ExtendedQT -lExtendedQT

    !isEmpty( PBVR_KVS_INSTALL_DIR ) {
        equals( KVS_SUPPORT_QT, "1" ) {
            LIBS += -L$$PBVR_KVS_INSTALL_DIR/lib -lkvsSupportQt -lkvsCore
        }
        equals( KVS_ENABLE_OPENGL, "1" ) {
            LIBS += -framework OpenGL
        }
        equals( KVS_SUPPORT_GLUT, "1" ) {
            LIBS += -framework GLUT
            LIBS += -L$$PBVR_KVS_INSTALL_DIR/lib -lkvsSupportGLUT
        }
    }
    !isEmpty( KVS_ASSIMP_DIR ) {
        LIBS += -L$$KVS_ASSIMP_DIR/lib -lassimp -lIrrXML -lzlibstatic
    }
}

unix:!macx {
    LIBS += -L$$CLIENT_DIR/Widgets -lWidgets
    LIBS += -L$$CLIENT_DIR/FunctionParser -lFunctionParser
    LIBS += -L$$CLIENT_DIR/Utils -lUtils
    LIBS += -L$$CLIENT_DIR/ExtendedKVS -lExtendedKVS
    LIBS += -L$$CLIENT_DIR/ExtendedQT -lExtendedQT

    !isEmpty( PBVR_KVS_INSTALL_DIR ) {
        equals( KVS_SUPPORT_QT, "1" ) {
            LIBS += -L$$PBVR_KVS_INSTALL_DIR/lib -lkvsSupportQt -lkvsCore
        }
        equals( KVS_ENABLE_OPENGL, "1" ) {
            LIBS += -lGL
        }
        equals( KVS_ENABLE_GLU, "1" ) {
            LIBS += -lGLU
        }
        equals( KVS_SUPPORT_GLUT, "1" ) {
            LIBS += -lglut
            LIBS += -L$$PBVR_KVS_INSTALL_DIR/lib -lkvsSupportGLUT
        }
    }
    !isEmpty( KVS_ASSIMP_DIR ) {
        LIBS += -L$$KVS_ASSIMP_DIR/lib -lassimp -lIrrXML -lzlibstatic
    }
    LIBS += -lstdc++fs
}

win32 {
    CONFIG(release, debug|release) {
        PRE_TARGETDEPS += $$CLIENT_DIR/Widgets/release/Widgets.lib
        PRE_TARGETDEPS += $$CLIENT_DIR/FunctionParser/release/FunctionParser.lib
        PRE_TARGETDEPS += $$CLIENT_DIR/Utils/release/Utils.lib
        PRE_TARGETDEPS += $$CLIENT_DIR/ExtendedKVS/release/ExtendedKVS.lib
        PRE_TARGETDEPS += $$CLIENT_DIR/ExtendedQT/release/ExtendedQT.lib
    } else:CONFIG(debug, debug|release) {
        PRE_TARGETDEPS += $$CLIENT_DIR/Widgets/debug/Widgets.lib
        PRE_TARGETDEPS += $$CLIENT_DIR/FunctionParser/debug/FunctionParser.lib
        PRE_TARGETDEPS += $$CLIENT_DIR/Utils/debug/Utils.lib
        PRE_TARGETDEPS += $$CLIENT_DIR/ExtendedKVS/debug/ExtendedKVS.lib
        PRE_TARGETDEPS += $$CLIENT_DIR/ExtendedQT/debug/ExtendedQT.lib
    }
}

macx {
    PRE_TARGETDEPS += $$CLIENT_DIR/Widgets/libWidgets.a
    PRE_TARGETDEPS += $$CLIENT_DIR/FunctionParser/libFunctionParser.a
    PRE_TARGETDEPS += $$CLIENT_DIR/Utils/libUtils.a
    PRE_TARGETDEPS += $$CLIENT_DIR/ExtendedKVS/libExtendedKVS.a
    PRE_TARGETDEPS += $$CLIENT_DIR/ExtendedQT/libExtendedQT.a
}

unix:!macx {
    PRE_TARGETDEPS += $$CLIENT_DIR/Widgets/libWidgets.a
    PRE_TARGETDEPS += $$CLIENT_DIR/FunctionParser/libFunctionParser.a
    PRE_TARGETDEPS += $$CLIENT_DIR/Utils/libUtils.a
    PRE_TARGETDEPS += $$CLIENT_DIR/ExtendedKVS/libExtendedKVS.a
    PRE_TARGETDEPS += $$CLIENT_DIR/ExtendedQT/libExtendedQT.a
}

include(../KvsRuntimeResources.pri)
