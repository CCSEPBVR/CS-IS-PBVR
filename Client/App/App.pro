include(../SETTINGS.pri)
#=============================================================================
#  Configuration valiable.
#=============================================================================
#CONFIG += release warn_off opengl
CONFIG += warn_off opengl
QT += opengl
QT += printsupport

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
TARGET = pbvr_client



#=============================================================================
#  Link library.
#=============================================================================
win32 {
LIBS += ws2_32.lib
CONFIG(release, debug|release) {
LIBS += -L../Widgets/release -lWidgets
LIBS += -L../FunctionParser/release -lpbvrFunc
LIBS += -L../Common/release -lCommon
LIBS += -L../ExtendedKVS/release -lExtendedKVS
LIBS += -L../ExtendedQT/release -lExtendedQT
} else:CONFIG(debug, debug|release) {
LIBS += -L../Widgets/debug -lWidgets
LIBS += -L../FunctionParser/debug -lpbvrFunc
LIBS += -L../Common/debug -lCommon
LIBS += -L../ExtendedKVS/debug -lExtendedKVS
LIBS += -L../ExtendedQT/debug -lExtendedQT
}
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
    LIBS += -lopengl32
    LIBS += -lglu32
    LIBS += $$GLEW_LIBRARY_PATH/glew32.lib

    COPY_GLEW_SRC = $$GLEW_BINARY_PATH
    COPY_GLEW_SRC ~= s|/|\|gi
    COPY_GLEW_DEST = $$OUT_PWD/release
    COPY_GLEW_DEST ~= s|/|\|gi
    copy_glew.target = copy_glew
    copy_glew.commands = $$QMAKE_COPY $$COPY_GLEW_SRC\glew32.dll $$COPY_GLEW_DEST
    QMAKE_EXTRA_TARGETS += copy_glew
    POST_TARGETDEPS += copy_glew
  }
  
  equals( KVS_SUPPORT_GLUT, "1" ) {
    LIBS += $$GLUT_LIBRARY_PATH/freeglut.lib
    LIBS += $$KVS_DIR/lib/kvsSupportGLUT.lib

    COPY_GLUT_SRC = $$GLUT_BINARY_PATH
    COPY_GLUT_SRC ~= s|/|\|gi
    COPY_GLUT_DEST = $$OUT_PWD/release
    COPY_GLUT_DEST ~= s|/|\|gi
    copy_glut.target = copy_glut
    copy_glut.commands = $$QMAKE_COPY $$COPY_GLUT_SRC\freeglut.dll $$COPY_GLUT_DEST
    QMAKE_EXTRA_TARGETS += copy_glut
    POST_TARGETDEPS += copy_glut
  }
  
  equals( KVS_SUPPORT_OPENXR, "1" ) {
    LIBS += $$KVS_DIR/lib/kvsSupportOpenXR.lib
    LIBS += $$OPENXR_LIBRARY_PATH/openxr_loader.lib
    LIBS += -lgdi32

    COPY_OPENXR_SRC = $$OPENXR_BINARY_PATH
    COPY_OPENXR_SRC ~= s|/|\|gi
    COPY_OPENXR_DEST = $$OUT_PWD/release
    COPY_OPENXR_DEST ~= s|/|\|gi
    copy_openxr.target = copy_openxr
    copy_openxr.commands += $$QMAKE_COPY $$COPY_OPENXR_SRC\openxr_loader.dll $$COPY_OPENXR_DEST
    QMAKE_EXTRA_TARGETS += copy_openxr
    POST_TARGETDEPS += copy_openxr
    
    COPY_MODEL_SRC = $$KVS_DIR/resources/SupportOpenXR/Models
    COPY_MODEL_SRC ~= s|/|\|gi
    copy_hand_model.target = copy_hand_model
    copy_hand_model.commands += $$QMAKE_COPY_DIR $$COPY_MODEL_SRC $$COPY_OPENXR_DEST\Models
    QMAKE_EXTRA_TARGETS += copy_hand_model
    POST_TARGETDEPS += copy_hand_model
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
        isEmpty( ASSIMP_LIB_DIR ) {
            error( "ASSIMP_LIB_DIR is not defined" );
        }else{
            LIBS += -L$$ASSIMP_LIB_DIR -lassimp -lIrrXML -lzlibstatic
        }
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
    resources.qrc



#=============================================================================
#  Pre Targetdeps
#=============================================================================
win32 {
CONFIG(release, debug|release) {
    PRE_TARGETDEPS += ../Widgets/release/Widgets.lib
    PRE_TARGETDEPS += ../FunctionParser/release/pbvrFunc.lib
    PRE_TARGETDEPS += ../Common/release/Common.lib
    PRE_TARGETDEPS += ../ExtendedKVS/release/ExtendedKVS.lib
    PRE_TARGETDEPS += ../ExtendedQT/release/ExtendedQT.lib
} else:CONFIG(debug, debug|release) {
    PRE_TARGETDEPS += ../Widgets/debug/Widgets.lib
    PRE_TARGETDEPS += ../FunctionParser/debug/pbvrFunc.lib
    PRE_TARGETDEPS += ../Common/debug/Common.lib
    PRE_TARGETDEPS += ../ExtendedKVS/debug/ExtendedKVS.lib
    PRE_TARGETDEPS += ../ExtendedQT/debug/ExtendedQT.lib
}
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
}

win32 {
CONFIG(release, debug|release) {
QMAKE_POST_LINK = rmdir /s /q .\release\Font & mkdir .\release\Font & xcopy "$$replace(FONT_DIR, /, \\)\*" .\release\Font & rmdir /s /q .\release\Shader & mkdir .\release\Shader & xcopy "$$replace(SHADER_DIR, /, \\)\*" .\release\Shader
} else:CONFIG(debug, debug|release) {
QMAKE_POST_LINK = rmdir /s /q .\debug\Font & mkdir .\debug\Font & xcopy "$$replace(FONT_DIR, /, \\)\*" .\debug\Font & rmdir /s /q .\debug\Shader & mkdir .\debug\Shader & xcopy "$$replace(SHADER_DIR, /, \\)\*" .\debug\Shader
}
}

macx {
QMAKE_POST_LINK = rm -rf pbvr_client.app/Contents/MacOS/Font && mkdir -p pbvr_client.app/Contents/MacOS/Font && cp -rf $$FONT_DIR/* pbvr_client.app/Contents/MacOS/Font/ && rm -rf pbvr_client.app/Contents/MacOS/Shader && mkdir -p pbvr_client.app/Contents/MacOS/Shader && cp -rf $$SHADER_DIR/* pbvr_client.app/Contents/MacOS/Shader/
}

unix:!macx {
QMAKE_POST_LINK = rm -rf Font && mkdir Font && cp -rf $$FONT_DIR/* Font && rm -rf Shader && mkdir Shader && cp -rf $$SHADER_DIR/* Shader
}
