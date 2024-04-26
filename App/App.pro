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

    contains(DEFINES, PBVR_SUPPORT_FBX) {
        isEmpty( FBXSDK_LIB_DIR ) {
            error( "FBXSDK_LIB_DIR is not defined" );
        }else{
            LIBS += -L$$FBXSDK_LIB_DIR -llibfbxsdk-mt -llibxml2-mt -lzlib-mt -ladvapi32
        }
    }
    contains(DEFINES, PBVR_SUPPORT_3DS) {
        isEmpty( ASSIMP_LIB_DIR ) {
            error( "ASSIMP_LIB_DIR is not defined" );
        }else{
            LIBS += -L$$ASSIMP_LIB_DIR -lassimp-vc143-mt
        }
    }


LIBS += $$KVS_DIR/lib/kvsSupportQt.lib
LIBS += $$KVS_DIR/lib/kvsCore.lib
    equals( KVS_ENABLE_GLEW, "1" ) {
        win32: LIBS += -lopengl32
        win32: LIBS += -lglu32
        LIBS += $$GLEW_LIBRARY_PATH/glew32.lib
    }
    equals( KVS_SUPPORT_GLUT, "1" ) {
        LIBS += $$KVS_DIR/lib/kvsSupportGLUT.lib
    }
}

macx {
LIBS += -L../Widgets -lWidgets
LIBS += -L../FunctionParser -lpbvrFunc
LIBS += -L../Common -lCommon
LIBS += -L../ExtendedKVS -lExtendedKVS
LIBS += -L../ExtendedQT -lExtendedQT

    contains(DEFINES, PBVR_SUPPORT_FBX) {
        isEmpty( FBXSDK_LIB_DIR ) {
            error( "FBXSDK_LIB_DIR is not defined" );
        }else{
            LIBS += -L$$FBXSDK_LIB_DIR -lfbxsdk
        }
    }
    contains(DEFINES, PBVR_SUPPORT_3DS) {
        isEmpty( ASSIMP_LIB_DIR ) {
            error( "ASSIMP_LIB_DIR is not defined" );
        }else{
            LIBS += -L$$ASSIMP_LIB_DIR -lassimp -lIrrXML -lzlibstatic
        }
    }

LIBS += $$KVS_DIR/lib/libkvsSupportQt.a
LIBS += $$KVS_DIR/lib/libkvsCore.a
    equals( KVS_ENABLE_GLEW, "1" ) {
        LIBS += -L$$GLEW_LIBRARY_PATH -lGLEW
    }
    equals( KVS_ENABLE_OPENMP, "1" ) {
        LIBS += -L$$OPENMP_LIBRARY_PATH
        macx-clang { LIBS += -lomp } else { LIBS += -lgomp }
    }
    equals( KVS_SUPPORT_GLUT, "1" ) {
        LIBS += $$KVS_DIR/lib/libkvsSupportGLUT.a -framework GLUT
    }
    equals( KVS_SUPPORT_FFMPEG, "1" ) {
        LIBS += $$KVS_DIR/lib/libkvsSupportFFmpeg.a -L$$FFMPEG_LIBRARY_PATH
        LIBS += -lavcodec -lavformat -lavutil -lavdevice -lavfilter -lswscale
    }
    equals( KVS_SUPPORT_OPENCV, "1" ) {
        LIBS += $$KVS_DIR/lib/libkvsSupportOpenCV.a $$OPENCV_LIBRARY_PATH
        LIBS += $$OPENCV_LINK_LIBRARY
    }
    equals( KVS_SUPPORT_PYTHON, "1" ) {
        LIBS += $$KVS_DIR/lib/libkvsSupportPython.a $$PYTHON_LIBRARY_PATH
        LIBS += $$PYTHON_LINK_LIBRARY
    }
}

unix:!macx {
LIBS += -L../Widgets -lWidgets
LIBS += -L../FunctionParser -lpbvrFunc
LIBS += -L../Common -lCommon
LIBS += -L../ExtendedKVS -lExtendedKVS
LIBS += -L../ExtendedQT -lExtendedQT

    contains(DEFINES, PBVR_SUPPORT_FBX) {
        isEmpty( FBXSDK_LIB_DIR ) {
            error( "FBXSDK_LIB_DIR is not defined" );
        }
        else{
            LIBS += -L$$FBXSDK_LIB_DIR -lfbxsdk
        }
    }
    contains(DEFINES, PBVR_SUPPORT_3DS) {
    }

LIBS += $$KVS_DIR/lib/libkvsSupportQt.a
LIBS += $$KVS_DIR/lib/libkvsCore.a
    equals( KVS_ENABLE_GLU, "1" ) {
        LIBS += -lGLU
    }
    equals( KVS_ENABLE_GLEW, "1" ) {
        LIBS += -L$$GLEW_LIBRARY_PATH -lGLEW
    }
    equals( KVS_ENABLE_OPENMP, "1" ) {
        LIBS += -L$$OPENMP_LIBRARY_PATH -lgomp -fopenmp
    }
    equals( KVS_SUPPORT_GLUT, "1" ) {
        LIBS += $$KVS_DIR/lib/libkvsSupportGLUT.a -lglut
    }
    equals( KVS_SUPPORT_FFMPEG, "1" ) {
        LIBS += $$KVS_DIR/lib/libkvsSupportFFmpeg.a $$FFMPEG_LIBRARY_PATH
        LIBS += -lavcodec -lavformat -lavutil -lavdevice -lavfilter -lswscale
    }
    equals( KVS_SUPPORT_OPENCV, "1" ) {
        LIBS += $$KVS_DIR/lib/libkvsSupportOpenCV.a $$OPENCV_LIBRARY_PATH
        LIBS += $$OPENCV_LINK_LIBRARY
    }
    equals( KVS_SUPPORT_PYTHON, "1" ) {
        LIBS += $$KVS_DIR/lib/libkvsSupportPython.a $$PYTHON_LIBRARY_PATH
        LIBS += $$PYTHON_LINK_LIBRARY
    }
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
