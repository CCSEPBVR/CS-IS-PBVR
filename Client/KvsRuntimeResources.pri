#=============================================================================
# KVS runtime resources
#=============================================================================
!isEmpty( PBVR_KVS_INSTALL_DIR ) {
    FONT_DIR = $$PBVR_KVS_INSTALL_DIR/include/Core/Visualization/Font
    SHADER_DIR = $$PBVR_KVS_INSTALL_DIR/include/Core/Visualization/Shader

    win32 {
        CONFIG(release, debug|release) {
            KVS_RUNTIME_DIR = $$OUT_PWD/release
        } else:CONFIG(debug, debug|release) {
            KVS_RUNTIME_DIR = $$OUT_PWD/debug
        }

        KVS_RUNTIME_DIR_WIN = $$replace(KVS_RUNTIME_DIR, /, \\)
        FONT_DIR_WIN = $$replace(FONT_DIR, /, \\)
        SHADER_DIR_WIN = $$replace(SHADER_DIR, /, \\)

        QMAKE_POST_LINK = \
            if exist "$$KVS_RUNTIME_DIR_WIN\\Font" rmdir /s /q "$$KVS_RUNTIME_DIR_WIN\\Font" & mkdir "$$KVS_RUNTIME_DIR_WIN\\Font" & xcopy "$$FONT_DIR_WIN\\*" "$$KVS_RUNTIME_DIR_WIN\\Font\\" /E /I /Y & \
            if exist "$$KVS_RUNTIME_DIR_WIN\\Shader" rmdir /s /q "$$KVS_RUNTIME_DIR_WIN\\Shader" & mkdir "$$KVS_RUNTIME_DIR_WIN\\Shader" & xcopy "$$SHADER_DIR_WIN\\*" "$$KVS_RUNTIME_DIR_WIN\\Shader\\" /E /I /Y

        equals( KVS_ENABLE_GLEW, "1" ) {
            COPY_GLEW_SRC = $$KVS_GLEW_DIR/bin
            COPY_GLEW_SRC ~= s|/|\\|g
            COPY_GLEW_DEST = $$KVS_RUNTIME_DIR
            COPY_GLEW_DEST ~= s|/|\\|g
            copy_glew.target = copy_glew
            copy_glew.commands = $$QMAKE_COPY $$COPY_GLEW_SRC\\glew32.dll $$COPY_GLEW_DEST
            QMAKE_EXTRA_TARGETS += copy_glew
            POST_TARGETDEPS += copy_glew
        }

        equals( KVS_SUPPORT_GLUT, "1" ) {
            COPY_GLUT_SRC = $$KVS_GLUT_DIR/bin
            COPY_GLUT_SRC ~= s|/|\\|g
            COPY_GLUT_DEST = $$KVS_RUNTIME_DIR
            COPY_GLUT_DEST ~= s|/|\\|g
            copy_glut.target = copy_glut
            copy_glut.commands = $$QMAKE_COPY $$COPY_GLUT_SRC\\freeglut.dll $$COPY_GLUT_DEST
            QMAKE_EXTRA_TARGETS += copy_glut
            POST_TARGETDEPS += copy_glut
        }

        equals( KVS_SUPPORT_OPENXR, "1" ) {
            COPY_OPENXR_SRC = $$KVS_OPENXR_DIR/bin
            COPY_OPENXR_SRC ~= s|/|\\|g
            COPY_OPENXR_DEST = $$KVS_RUNTIME_DIR
            COPY_OPENXR_DEST ~= s|/|\\|g
            copy_openxr.target = copy_openxr
            copy_openxr.commands = $$QMAKE_COPY $$COPY_OPENXR_SRC\\openxr_loader.dll $$COPY_OPENXR_DEST
            QMAKE_EXTRA_TARGETS += copy_openxr
            POST_TARGETDEPS += copy_openxr

            COPY_MODEL_SRC = $$PBVR_KVS_INSTALL_DIR/resources/SupportOpenXR/Models
            COPY_MODEL_SRC ~= s|/|\\|g
            copy_hand_model.target = copy_hand_model
            copy_hand_model.commands = $$QMAKE_COPY_DIR $$COPY_MODEL_SRC $$COPY_OPENXR_DEST\\Models
            QMAKE_EXTRA_TARGETS += copy_hand_model
            POST_TARGETDEPS += copy_hand_model
        }
    }

    macx {
        KVS_RUNTIME_DIR = $$OUT_PWD/$${TARGET}.app/Contents/MacOS
        QMAKE_POST_LINK = rm -rf "$$KVS_RUNTIME_DIR/Font" && mkdir -p "$$KVS_RUNTIME_DIR/Font" && cp -rf "$$FONT_DIR/"* "$$KVS_RUNTIME_DIR/Font/" && \
                          rm -rf "$$KVS_RUNTIME_DIR/Shader" && mkdir -p "$$KVS_RUNTIME_DIR/Shader" && cp -rf "$$SHADER_DIR/"* "$$KVS_RUNTIME_DIR/Shader/"
    }

    unix:!macx {
        KVS_RUNTIME_DIR = $$OUT_PWD
        QMAKE_POST_LINK = rm -rf "$$KVS_RUNTIME_DIR/Font" && mkdir -p "$$KVS_RUNTIME_DIR/Font" && cp -rf "$$FONT_DIR/"* "$$KVS_RUNTIME_DIR/Font/" && \
                          rm -rf "$$KVS_RUNTIME_DIR/Shader" && mkdir -p "$$KVS_RUNTIME_DIR/Shader" && cp -rf "$$SHADER_DIR/"* "$$KVS_RUNTIME_DIR/Shader/"
    }
}
