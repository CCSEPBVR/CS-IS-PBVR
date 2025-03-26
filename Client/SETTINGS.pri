include(qtpbvr.conf)
#=============================================================================
#  KVS directory.
#=============================================================================
KVS_DIR=$$(KVS_DIR)
isEmpty( KVS_DIR ) {
    error( "KVS_DIR is not defined." )
}
else {
    include( $$KVS_DIR/kvs.conf )
}

FONT_DIR=$$PWD/Font
SHADER_DIR=$$PWD/Shader

#=============================================================================
#  Compiler.
#=============================================================================
KVS_CC = $$(KVS_CC)
KVS_CPP = $$(KVS_CPP)

!isEmpty( KVS_CC ) {
    QMAKE_CC = $(KVS_CC)
    QMAKE_LINK_C = $(KVS_CC)
    QMAKE_LINK_C_SHLIB = $(KVS_CC)
}

!isEmpty( KVS_CPP ) {
    QMAKE_CXX = $(KVS_CPP)
    QMAKE_LINK = $(KVS_CPP)
    QMAKE_LINK_SHLIB = $(KVS_CPP)
}

#=============================================================================
#  Configuration valiable.
#=============================================================================
CONFIG(release, debug|release) {
    CONFIG += release
    win32 {
        QMAKE_CXXFLAGS_RELEASE -= -O2
        QMAKE_CXXFLAGS_RELEASE += /Ox
        QMAKE_CFLAGS_RELEASE += /MT
        QMAKE_CXXFLAGS_RELEASE += /MT
    }
    macx {
        CONFIG += optimize_full
    }
    unix {
        CONFIG += optimize_full
    }
}

# デバッグモードの設定
CONFIG(debug, debug|release) {
    CONFIG += debug
    win32 {
        QMAKE_CXXFLAGS_DEBUG += /Od /DDEBUG
        #When using KVS with DEBUG=1, please use MTd.
        QMAKE_CFLAGS_DEBUG += /MT
        QMAKE_CXXFLAGS_DEBUG += /MT
        # QMAKE_CFLAGS_DEBUG += /MTd
        # QMAKE_CXXFLAGS_DEBUG += /MTd
    }
    macx {
        QMAKE_CXXFLAGS_DEBUG += -O0 -DDEBUG
    }
    unix {
        QMAKE_CXXFLAGS_DEBUG += -O0 -DDEBUG
    }
}
equals( KVS_SUPPORT_PYTHON, "1" ) { CONFIG += no_keywords }


#=============================================================================
#  Definition.
#=============================================================================
win32 {
    DEFINES += WIN32 _MBCS NOMINMAX _SCL_SECURE_NO_DEPRECATE _CRT_SECURE_NO_DEPRECATE _CRT_NONSTDC_NO_DEPRECATE
}

!isEmpty( DEBUG ) {
    DEFINES += _DEBUG KVS_ENABLE_DEBUG
} else {
    DEFINES += NDEBUG
}

#!isEmpty( MEM_DEBUG )
#{
#    DEFINES += KVS_ENABLE_MEM_DEBUG
#}
DEFINES += PBVR_VERSION=\\\"3.3.0\\\"
equals( KVS_ENABLE_OPENGL, "1" ) { DEFINES += KVS_ENABLE_OPENGL }
equals( KVS_ENABLE_GLU, "1" ) { DEFINES += KVS_ENABLE_GLU }
equals( KVS_ENABLE_GLEW, "1" ) { DEFINES += KVS_ENABLE_GLEW }
equals( KVS_ENABLE_FFMPEG, "1" ) { DEFINES += KVS_ENABLE_FFMPEG }
equals( KVS_ENABLE_OPENMP, "1" ) { DEFINES += KVS_ENABLE_OPENMP }
equals( KVS_ENABLE_DEPRECATED, "1" ) { DEFINES += KVS_ENABLE_DEPRECATED }
equals( KVS_SUPPORT_GLUT, "1" ) { DEFINES += KVS_SUPPORT_GLUT }
equals( KVS_SUPPORT_QT, "1" ) { DEFINES += KVS_SUPPORT_QT }
equals( KVS_SUPPORT_OPENXR,"1" ) { DEFINES += KVS_SUPPORT_OPENXR }
equals( KVS_SUPPORT_OPENCV, "1" ) { DEFINES += KVS_SUPPORT_OPENCV }
equals( KVS_SUPPORT_CUDA, "1" ) { DEFINES += KVS_SUPPORT_CUDA }
equals( KVS_SUPPORT_PYTHON, "1" ) { DEFINES += KVS_SUPPORT_PYTHON }
equals( KVS_SUPPORT_EGL, "1" ) { DEFINES += KVS_SUPPORT_EGL }

equals( DESKTOP_SCREEN_MODE, "1" ) { DEFINES += DESKTOP_SCREEN_MODE }
equals( OPENXR_SCREEN_MODE, "1" ) { DEFINES += OPENXR_SCREEN_MODE }

equals( PBVR_SUPPORT_FBX, "1" ) { DEFINES += PBVR_SUPPORT_FBX }
equals( PBVR_SUPPORT_3DS, "1" ) { DEFINES += PBVR_SUPPORT_3DS }

DEFINES += ADDITIONAL_SHADER_DIR=\\\"$$PWD/Shader\\\"
DEFINES += JSON_DIR=\\\"$$PWD/json\\\"


#=============================================================================
#  Include path.
#=============================================================================
INCLUDEPATH += $$KVS_DIR/include

# GLUT environment variables
KVS_GLUT_DIR = $$(KVS_GLUT_DIR)
KVS_GLUT_INCLUDE_PATH = $$(KVS_GLUT_INCLUDE_PATH)
KVS_GLUT_LIBRARY_PATH = $$(KVS_GLUT_LIBRARY_PATH)
KVS_GLUT_BINARY_PATH = $$(KVS_GLUT_BINARY_PATH)

GLUT_INCLUDE_PATH = .
GLUT_LIBRARY_PATH = .
GLUT_BINARY_PATH = .
equals( KVS_SUPPORT_GLUT, "1" ) {
    !isEmpty( KVS_GLUT_DIR ) {
        GLUT_INCLUDE_PATH = $$KVS_GLUT_DIR/include
        GLUT_LIBRARY_PATH = $$KVS_GLUT_DIR/lib
        GLUT_BINARY_PATH = $$KVS_GLUT_DIR/bin
    } else {
        !isEmpty( KVS_GLUT_INCLUDE_PATH ) {
            GLUT_INCLUDE_PATH = $$KVS_GLUT_INCLUDE_PATH
        }
        !isEmpty( KVS_GLEW_LIBRARY_PATH ) {
            GLUT_LIBRARY_PATH = $$KVS_GLUT_LIBRARY_PATH
        }
        !isEmpty( KVS_GLUT_BINARY_PATH ) {
            GLUT_BINARY_PATH = $$KVS_GLUT_BINARY_PATH
        }
    }
}

INCLUDEPATH += $$GLUT_INCLUDE_PATH

# GLEW environment variables
KVS_GLEW_DIR = $$(KVS_GLEW_DIR)
KVS_GLEW_INCLUDE_PATH = $$(KVS_GLEW_INCLUDE_PATH)
KVS_GLEW_LIBRARY_PATH = $$(KVS_GLEW_LIBRARY_PATH)
KVS_GLEW_BINARY_PATH = $$(KVS_GLEW_BINARY_PATH)

GLEW_INCLUDE_PATH = .
GLEW_LIBRARY_PATH = .
GLEW_BINARY_PATH = .
equals( KVS_ENABLE_GLEW, "1" ) {
    !isEmpty( KVS_GLEW_DIR ) {
        GLEW_INCLUDE_PATH = $$KVS_GLEW_DIR/include
        GLEW_LIBRARY_PATH = $$KVS_GLEW_DIR/lib
        GLEW_BINARY_PATH = $$KVS_GLEW_DIR/bin
    } else {
        !isEmpty( KVS_GLEW_INCLUDE_PATH ) {
            GLEW_INCLUDE_PATH = $$KVS_GLEW_INCLUDE_PATH
        }
        !isEmpty( KVS_GLEW_LIBRARY_PATH ) {
            GLEW_LIBRARY_PATH = $$KVS_GLEW_LIBRARY_PATH
        }
        !isEmpty( KVS_GLEW_BINARY_PATH ) {
            GLEW_BINARY_PATH = $$KVS_GLEW_BINARY_PATH
        }
    }
}

INCLUDEPATH += $$GLEW_INCLUDE_PATH

# OPENXR environment variables
KVS_OPENXR_DIR = $$(KVS_OPENXR_DIR)
KVS_OPENXR_INCLUDE_PATH = $$(KVS_OPENXR_INCLUDE_PATH)
KVS_OPENXR_LIBRARY_PATH = $$(KVS_OPENXR_LIBRARY_PATH)
KVS_OPENXR_BINARY_PATH = $$(KVS_OPENXR_BINARY_PATH)

KVS_IMGUI_DIR = $$(KVS_IMGUI_DIR)
KVS_IMGUI_INCLUDE_PATH = $$(KVS_IMGUI_INCLUDE_PATH)

OPENXR_INCLUDE_PATH = .
OPENXR_LIBRARY_PATH = .
OPENXR_BINARY_PATH = .
IMGUI_INCLUDE_PATH = .
equals( KVS_SUPPORT_OPENXR, "1" ) {
    !isEmpty( KVS_OPENXR_DIR ) {
        OPENXR_INCLUDE_PATH = $$KVS_OPENXR_DIR/include
        OPENXR_LIBRARY_PATH = $$KVS_OPENXR_DIR/lib
        OPENXR_BINARY_PATH = $$KVS_OPENXR_DIR/bin
    } else {
        !isEmpty( KVS_OPENXR_INCLUDE_PATH ) {
            OPENXR_INCLUDE_PATH = $$KVS_OPENXR_INCLUDE_PATH
        }
        !isEmpty( KVS_OPENXR_LIBRARY_PATH ) {
            OPENXR_LIBRARY_PATH = $$KVS_OPENXR_LIBRARY_PATH
        }
        !isEmpty( KVS_OPENXR_BINARY_PATH ) {
            OPENXR_BINARY_PATH = $$KVS_OPENXR_BINARY_PATH
        }
    }
    !isEmpty( KVS_IMGUI_DIR ) {
        IMGUI_INCLUDE_PATH = $$KVS_IMGUI_DIR/include
    } else {
        !isEmpty( KVS_IMGUI_INCLUDE_PATH ) {
            IMGUI_INCLUDE_PATH = $$KVS_IMGUI_INCLUDE_PATH
        }
    }
}

INCLUDEPATH += $$OPENXR_INCLUDE_PATH
INCLUDEPATH += $$IMGUI_INCLUDE_PATH

# OpenMP environment variables
KVS_OPENMP_DIR = $$(KVS_OPENMP_DIR)
KVS_OPENMP_INCLUDE_PATH = $$(KVS_OPENMP_INCLUDE_PATH)
KVS_OPENMP_LIBRARY_PATH = $$(KVS_OPENMP_LIBRARY_PATH)

OPENMP_INCLUDE_PATH = .
OPENMP_LIBRARY_PATH = .
equals( KVS_ENABLE_OPENMP, "1" ) {
    !isEmpty( KVS_OPENMP_DIR ) {
        OPENMP_INCLUDE_PATH = $$KVS_OPENMP_DIR/include
        OPENMP_LIBRARY_PATH = $$KVS_OPENMP_DIR/lib
    } else {
        !isEmpty( KVS_OPENMP_INCLUDE_PATH ) {
            OPENMP_INCLUDE_PATH = $$KVS_OPENMP_INCLUDE_PATH
        }
        !isEmpty( KVS_OPENMP_LIBRARY_PATH ) {
            OPENMP_LIBRARY_PATH = $$KVS_OPENMP_LIBRARY_PATH
        }
    }
}

INCLUDEPATH += $$OPENMP_INCLUDE_PATH

# OpenCV environment variables
KVS_OPENCV_DIR = $$(KVS_OPENCV_DIR)

OPENCV_INCLUDE_PATH =
OPENCV_LIBRARY_PATH =
OPENCV_LINK_LIBRARY =
equals( KVS_SUPPORT_OPENCV, "1" ) {
    !isEmpty( KVS_OPENCV_DIR ) {
        OPENCV_INCLUDE_PATH = $$KVS_OPENCV_DIR/include
        OPENCV_LIBRARY_PATH = $$KVS_OPENCV_DIR/lib
    } else {
        OPENCV4 = $$system( "pkg-config --modversion opencv4" )
        contains( OPENCV4, "not" ) {
            OPENCV_INCLUDE_PATH = $$system( "pkg-config --cflags opencv | sed -e 's/-I//g'")
            OPENCV_LIBRARY_PATH = $$system( "pkg-config --libs-only-L opencv" )
            OPENCV_LINK_LIBRARY = $$system( "pkg-config --libs opencv" )
        } else {
            OPENCV_INCLUDE_PATH = $$system( "pkg-config --cflags opencv4 | sed -e 's/-I//g'")
            OPENCV_LIBRARY_PATH = $$system( "pkg-config --libs-only-L opencv4" )
            OPENCV_LINK_LIBRARY = $$system( "pkg-config --libs opencv4" )
            DEFINES += KVS_OPENCV4
        }
    }
}

INCLUDEPATH += $$OPENCV_INCLUDE_PATH

# FFmpeg environment variables
KVS_FFMPEG_DIR = $$(KVS_FFMPEG_DIR)

FFMPEG_INCLUDE_PATH =
FFMPEG_LIBRARY_PATH =
FFMPEG_LINK_LIBRARY =
equals( KVS_SUPPORT_FFMPEG, "1" ) {
    !isEmpty( KVS_FFMPEG_DIR ) {
        FFMPEG_INCLUDE_PATH = $$KVS_FFMPEG_DIR/include
        FFMPEG_LIBRARY_PATH = $$KVS_FFMPEG_DIR/lib
    } else {
        !isEmpty( KVS_FFMPEG_INCLUDE_PATH ) {
            FFMPEG_INCLUDE_PATH = $$KVS_FFMPEG_INCLUDE_PATH
        }
        !isEmpty( KVS_FFMPEG_LIBRARY_PATH ) {
            FFMPEG_LIBRARY_PATH = $$KVS_FFMPEG_LIBRARY_PATH
        }
    }
}

INCLUDEPATH += $$FFMPEG_INCLUDE_PATH

# Python environment variables
KVS_PYTHON_DIR = $$(KVS_PYTHON_DIR)
KVS_NUMPY_DIR = $$(KVS_NUMPY_DIR)

KVS_PY = $$(KVS_PY)
isEmpty( KVS_PY ) {
    PY = python
    PYCONF = python-config
}
else {
    PY = $KVS_PY
    PYCONF = $KVS_PY-config
}

PYTHON_INCLUDE_PATH = .
PYTHON_LIBRARY_PATH = .
PYTHON_LINK_LIBRARY =
NUMPY_INCLUDE_PATH = .
NUMPY_LIBRARY_PATH = .
equals( KVS_SUPPORT_PYTHON, "1" ) {
    PYTHON_VER = $$system( "$$PY -c \"from __future__ import print_function; import sys; print( sys.version_info[0] )\"")
    PYTHON_INCLUDE_PATH = $$system( "$$PYCONF --includes | sed -e 's/-I//g'" )
    PYTHON_LIBRARY_PATH = $$system( "$$PYCONF --ldflags" )
    equals( PYTHON_VER, "2" ) {
        PYTHON_LINK_LIBRARY = $$system( "$$PYCONF --libs" )
    }
    equals( PYTHON_VER, "3" ) {
        PYTHON_LINK_LIBRARY = $$system( "$$PYCONF --libs --embed" )
    }
    !isEmpty( KVS_PYTHON_DIR ) {
        PYTHON_INCLUDE_PATH = $$KVS_PYTHON_DIR/include
        PYTHON_LIBRARY_PATH = $$KVS_PYTHON_DIR/lib
    }
    NUMPY_INCLUDE_PATH = $$system( "$$PY -c \"import numpy; print( numpy.get_include() )\"" )
    !isEmpty( KVS_NUMPY_DIR ) {
        NUMPY_INCLUDE_PATH = $$KVS_NUMPY_DIR/include
        NUMPY_LIBRARY_PATH = $$KVS_NUMPY_DIR/lib
    }
}

INCLUDEPATH += $$PYTHON_INCLUDE_PATH
INCLUDEPATH += $$NUMPY_INCLUDE_PATH

contains(DEFINES, PBVR_SUPPORT_FBX) {
    isEmpty( FBXSDK_INC_DIR ) {
        error( "FBXSDK_INC_DIR is not defined" );
    }
    else{
        INCLUDEPATH += $$FBXSDK_INC_DIR
    }
}
contains(DEFINES, PBVR_SUPPORT_3DS) {
    isEmpty( ASSIMP_INC_DIR ) {
        error( "ASSIMP_INC_DIR is not defined" );
    }
    else{
        INCLUDEPATH += $$ASSIMP_INC_DIR
    }
}

INCLUDEPATH += ../
INCLUDEPATH += ../Common/
INCLUDEPATH += ../FunctionParser


#=============================================================================
#  Include.
#=============================================================================
exists( kvsmake.qt.conf ) { include( kvsmake.qt.conf ) }

#=============================================================================
#  Flags.
#=============================================================================
macx-clang
{
    equals( KVS_ENABLE_OPENMP, "1" ) {
        QMAKE_LFLAGS += -Xpreprocessor -fopenmp
    }
}
