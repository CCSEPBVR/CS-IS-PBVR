include(../SETTINGS.pri)
#=============================================================================
#  Configuration valiable.
#=============================================================================

#=============================================================================
#  Template.
#=============================================================================
TEMPLATE = app

#=============================================================================
#  Target.
#=============================================================================
TARGET = pbvr_client

#=============================================================================
#  Link library.
#=============================================================================
win32 {
    LIBS += ws2_32.lib
    CONFIG(release, debug|release){
        LIBS += -L../Widgets/release -lWidgets
        LIBS += -L../FunctionParser/release -lFunctionParser
        LIBS += -L../Utils/release -lUtils
        LIBS += -L../ExtendedKVS/release -lExtendedKVS
        LIBS += -L../ExtendedQT/release -lExtendedQT
    }
    else:CONFIG(debug, debug|release){
        LIBS += -L../Widgets/debug -lWidgets
        LIBS += -L../FunctionParser/debug -lFunctionParser
        LIBS += -L../Utils/debug -lUtils
        LIBS += -L../ExtendedKVS/debug -lExtendedKVS
        LIBS += -L../ExtendedQT/debug -lExtendedQT
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
    LIBS += -L../Widgets -lWidgets
    LIBS += -L../FunctionParser -lFunctionParser
    LIBS += -L../Utils -lUtils
    LIBS += -L../ExtendedKVS -lExtendedKVS
    LIBS += -L../ExtendedQT -lExtendedQT

    !isEmpty( PBVR_KVS_INSTALL_DIR ) {
        equals( KVS_SUPPORT_QT, "1" ) {
            LIBS += -L$$PBVR_KVS_INSTALL_DIR/lib -lkvsSupportQt -lkvsCore
        }
        equals( KVS_ENABLE_OPENGL, "1" ) {
            LIBS += -framework OpenGL
        }
        equals( KVS_ENABLE_GLU, "1" ) {
        }
        equals( KVS_ENABLE_GLEW, "1" ) {
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
    LIBS += -L../Widgets -lWidgets
    LIBS += -L../FunctionParser -lFunctionParser
    LIBS += -L../Utils -lUtils
    LIBS += -L../ExtendedKVS -lExtendedKVS
    LIBS += -L../ExtendedQT -lExtendedQT

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
        equals( KVS_ENABLE_GLEW, "1" ) {
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

#=============================================================================
#  Header.
#=============================================================================
HEADERS += \
    MainWindow.h

#=============================================================================
#  Source.
#=============================================================================
SOURCES += \
    MainWindow.cpp \
    main.cpp

#=============================================================================
#  Forms.
#=============================================================================
FORMS += \
    MainWindow.ui

#=============================================================================
#  Resource.
#=============================================================================
RESOURCES += \
    resources.qrc

#=============================================================================
#  Pre Targetdeps
#=============================================================================
win32 {
    CONFIG(release, debug|release) {
        PRE_TARGETDEPS += ../Widgets/release/Widgets.lib
        PRE_TARGETDEPS += ../FunctionParser/release/FunctionParser.lib
        PRE_TARGETDEPS += ../Utils/release/Utils.lib
        PRE_TARGETDEPS += ../ExtendedKVS/release/ExtendedKVS.lib
        PRE_TARGETDEPS += ../ExtendedQT/release/ExtendedQT.lib
    } else:CONFIG(debug, debug|release) {
        PRE_TARGETDEPS += ../Widgets/debug/Widgets.lib
        PRE_TARGETDEPS += ../FunctionParser/debug/FunctionParser.lib
        PRE_TARGETDEPS += ../Utils/debug/Utils.lib
        PRE_TARGETDEPS += ../ExtendedKVS/debug/ExtendedKVS.lib
        PRE_TARGETDEPS += ../ExtendedQT/debug/ExtendedQT.lib
    }
}

macx {
    PRE_TARGETDEPS += ../Widgets/libWidgets.a
    PRE_TARGETDEPS += ../FunctionParser/libFunctionParser.a
    PRE_TARGETDEPS += ../Utils/libUtils.a
    PRE_TARGETDEPS += ../ExtendedKVS/libExtendedKVS.a
    PRE_TARGETDEPS += ../ExtendedQT/libExtendedQT.a
}

unix:!macx {
    PRE_TARGETDEPS += ../Widgets/libWidgets.a
    PRE_TARGETDEPS += ../FunctionParser/libFunctionParser.a
    PRE_TARGETDEPS += ../Utils/libUtils.a
    PRE_TARGETDEPS += ../ExtendedKVS/libExtendedKVS.a
    PRE_TARGETDEPS += ../ExtendedQT/libExtendedQT.a
}

include(../KvsRuntimeResources.pri)
