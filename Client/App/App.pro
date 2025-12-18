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

    !isEmpty( KVS_DIR ) {
        equals( KVS_SUPPORT_QT, "1" ) {
            LIBS += -L$$KVS_DIR/lib -lkvsSupportQt -lkvsCore
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
            LIBS += -L$$KVS_DIR/lib -lkvsSupportGLUT
        }
        equals( KVS_SUPPORT_OPENXR, "1" ) {
            LIBS += -L$$KVS_DIR/lib -lkvsSupportOpenXR
            LIBS += -L$$KVS_OPENXR_DIR/lib -lopenxr_loader
            LIBS += -lgdi32
        }
    }
    !isEmpty( KVS_ASSIMP_DIR ) {
        LIBS += -L$$KVS_ASSIMP_DIR/lib/release -lassimp-vc143-mt
    }
}

macx {
    LIBS += -L../Widgets -lWidgets
    LIBS += -L../FunctionParser -lFunctionParser
    LIBS += -L../Utils -lUtils
    LIBS += -L../ExtendedKVS -lExtendedKVS
    LIBS += -L../ExtendedQT -lExtendedQT

    !isEmpty( KVS_DIR ) {
        equals( KVS_SUPPORT_QT, "1" ) {
            LIBS += -L$$KVS_DIR/lib -lkvsSupportQt -lkvsCore
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
            LIBS += -L$$KVS_DIR/lib -lkvsSupportGLUT
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

    !isEmpty( KVS_DIR ) {
        equals( KVS_SUPPORT_QT, "1" ) {
            LIBS += -L$$KVS_DIR/lib -lkvsSupportQt -lkvsCore
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
            LIBS += -L$$KVS_DIR/lib -lkvsSupportGLUT
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

#=============================================================================
# QMAKE_POST_LINK
#=============================================================================
!isEmpty( KVS_DIR ) {
    FONT_DIR = $$KVS_DIR/include/Core/Visualization/Font
    SHADER_DIR = $$KVS_DIR/include/Core/Visualization/Shader

    win32 {
        CONFIG(release, debug|release) {
            QMAKE_POST_LINK = \
                rmdir /s /q .\release\Font & mkdir .\release\Font & xcopy "$$replace(FONT_DIR, /, \\)\*" .\release\Font & \
                rmdir /s /q .\release\Shader & mkdir .\release\Shader & xcopy "$$replace(SHADER_DIR, /, \\)\*" .\release\Shader
        } else:CONFIG(debug, debug|release) {
            QMAKE_POST_LINK = \
                rmdir /s /q .\debug\Font & mkdir .\debug\Font & xcopy "$$replace(FONT_DIR, /, \\)\*" .\debug\Font & \
                rmdir /s /q .\debug\Shader & mkdir .\debug\Shader & xcopy "$$replace(SHADER_DIR, /, \\)\*" .\debug\Shader
        }

        # Windows向け DLL・リソースのコピー（QMAKE_EXTRA_TARGETSで管理）
        equals( KVS_ENABLE_GLEW, "1" ) {
            COPY_GLEW_SRC = $$KVS_GLEW_DIR/bin
            COPY_GLEW_SRC ~= s|/|\\|g
            CONFIG(release, debug|release) {
                COPY_GLEW_DEST = $$OUT_PWD/release
            } else:CONFIG(debug, debug|release) {
                COPY_GLEW_DEST = $$OUT_PWD/debug
            }
            COPY_GLEW_DEST ~= s|/|\\|g
            copy_glew.target = copy_glew
            copy_glew.commands = $$QMAKE_COPY $$COPY_GLEW_SRC\\glew32.dll $$COPY_GLEW_DEST
            QMAKE_EXTRA_TARGETS += copy_glew
            POST_TARGETDEPS += copy_glew
        }

        equals( KVS_SUPPORT_GLUT, "1" ) {
            COPY_GLUT_SRC = $$KVS_GLUT_DIR/bin
            COPY_GLUT_SRC ~= s|/|\\|g
            CONFIG(release, debug|release) {
                COPY_GLUT_DEST = $$OUT_PWD/release
            } else:CONFIG(debug, debug|release) {
                COPY_GLUT_DEST = $$OUT_PWD/debug
            }
            COPY_GLUT_DEST ~= s|/|\\|g
            copy_glut.target = copy_glut
            copy_glut.commands = $$QMAKE_COPY $$COPY_GLUT_SRC\\freeglut.dll $$COPY_GLUT_DEST
            QMAKE_EXTRA_TARGETS += copy_glut
            POST_TARGETDEPS += copy_glut
        }

        equals( KVS_SUPPORT_OPENXR, "1" ) {
            COPY_OPENXR_SRC = $$KVS_OPENXR_DIR/bin
            COPY_OPENXR_SRC ~= s|/|\\|g
            CONFIG(release, debug|release) {
                COPY_OPENXR_DEST = $$OUT_PWD/release
            } else:CONFIG(debug, debug|release) {
                COPY_OPENXR_DEST = $$OUT_PWD/debug
            }
            COPY_OPENXR_DEST ~= s|/|\\|g
            copy_openxr.target = copy_openxr
            copy_openxr.commands = $$QMAKE_COPY $$COPY_OPENXR_SRC\\openxr_loader.dll $$COPY_OPENXR_DEST
            QMAKE_EXTRA_TARGETS += copy_openxr
            POST_TARGETDEPS += copy_openxr

            COPY_MODEL_SRC = $$KVS_DIR/resources/SupportOpenXR/Models
            COPY_MODEL_SRC ~= s|/|\\|g
            copy_hand_model.target = copy_hand_model
            copy_hand_model.commands = $$QMAKE_COPY_DIR $$COPY_MODEL_SRC $$COPY_OPENXR_DEST\\Models
            QMAKE_EXTRA_TARGETS += copy_hand_model
            POST_TARGETDEPS += copy_hand_model
        }
    }

    macx {
        QMAKE_POST_LINK = rm -rf pbvr_client.app/Contents/MacOS/Font && mkdir -p pbvr_client.app/Contents/MacOS/Font && cp -rf $$FONT_DIR/* pbvr_client.app/Contents/MacOS/Font/ && \
                          rm -rf pbvr_client.app/Contents/MacOS/Shader && mkdir -p pbvr_client.app/Contents/MacOS/Shader && cp -rf $$SHADER_DIR/* pbvr_client.app/Contents/MacOS/Shader/
    }

    unix:!macx {
        QMAKE_POST_LINK = rm -rf Font && mkdir Font && cp -rf $$FONT_DIR/* Font && \
                          rm -rf Shader && mkdir Shader && cp -rf $$SHADER_DIR/* Shader
    }
}
