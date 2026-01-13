# ===============================
# ビルド構成
# ===============================
CONFIG += c++17            # C++17 標準でコンパイル
CONFIG += opengl           # OpenGL を有効化
CONFIG += warn_off         # ※警告を抑制（必要ならコメントアウト解除）

# macOSで std::filesystem を有効にする
macx {
    QMAKE_MACOSX_DEPLOYMENT_TARGET = 10.15
}

# ビルドタイプ別の最適化フラグ
CONFIG( release, debug|release ) {
    CONFIG += release

    win32 {
        QMAKE_CXXFLAGS_RELEASE -= -O2
        QMAKE_CXXFLAGS_RELEASE += /Ox
        QMAKE_CFLAGS_RELEASE += /MT
        QMAKE_CXXFLAGS_RELEASE += /MT
    }

    macx {
        QMAKE_CXXFLAGS_RELEASE -= -O2
        QMAKE_CXXFLAGS_RELEASE += -O3
    }

    unix:!macx {
        QMAKE_CXXFLAGS_RELEASE -= -O2
        QMAKE_CXXFLAGS_RELEASE += -O3
    }
}
else:CONFIG( debug, debug|release ) {
    CONFIG += debug

    win32 {
        QMAKE_CXXFLAGS_DEBUG += /Od /DDEBUG
        QMAKE_CFLAGS_DEBUG += /MTd
        QMAKE_CXXFLAGS_DEBUG += /MTd
    }

    macx {
        QMAKE_CXXFLAGS_DEBUG += -O0 -g -DDEBUG
    }

    unix:!macx {
        QMAKE_CXXFLAGS_DEBUG += -O0 -g -DDEBUG
    }
}

# ===============================
# Qt6 モジュール
# ===============================
QT += core
QT += gui
QT += widgets              # QWidgetベースのUIに必要
QT += opengl               # OpenGL サポート（QOpenGLFunctionsなど）
QT += openglwidgets        # QOpenGLWidget など
QT += printsupport         # QPrinter などの印刷機能
QT += websockets           # QWebSocket

# ===============================
# 非推奨APIの禁止（必要に応じて有効化）
# ===============================
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000

# ===============================
# インストールパス設定
# ===============================
unix:!android {
    target.path = /opt/$${TARGET}/bin
}

!isEmpty(target.path): INSTALLS += target

KVS_DIR         = $$(KVS_DIR)
KVS_GLEW_DIR    = $$(KVS_GLEW_DIR)
KVS_GLUT_DIR    = $$(KVS_GLUT_DIR)
KVS_OPENXR_DIR  = $$(KVS_OPENXR_DIR)
KVS_IMGUI_DIR   = $$(KVS_IMGUI_DIR)
KVS_ASSIMP_DIR  = $$(KVS_ASSIMP_DIR)

isEmpty( KVS_DIR ) {
    error( "The environment variable KVS_DIR is not defined." )
}
else {
    include( $$KVS_DIR/kvs.conf )
    win32 {
        DEFINES += WIN32 _MBCS NOMINMAX _SCL_SECURE_NO_DEPRECATE _CRT_SECURE_NO_DEPRECATE _CRT_NONSTDC_NO_DEPRECATE
    }
    !isEmpty( DEBUG ) {
        DEFINES += _DEBUG KVS_ENABLE_DEBUG
    } else {
        DEFINES += NDEBUG
    }
    INCLUDEPATH += $$KVS_DIR/include

    equals( KVS_ENABLE_OPENGL, "1" ) {  # すべてのプラットフォームで必要
        message( "KVS_ENABLE_OPENGL" )
        DEFINES += KVS_ENABLE_OPENGL
    }

    equals( KVS_ENABLE_GLU, "1" ){      # すべてのプラットフォームで必要
        message( "KVS_ENABLE_GLU" )
        DEFINES += KVS_ENABLE_GLU
    }

    equals( KVS_ENABLE_GLEW, "1" ){     # Windowsのみ必要
        message( "KVS_ENABLE_GLEW" )
        DEFINES += KVS_ENABLE_GLEW
        INCLUDEPATH += $$KVS_GLEW_DIR/include
    }

    equals( KVS_SUPPORT_GLUT, "1" ){    # すべてのプラットフォームで必要(?)
        message( "KVS_SUPPORT_GLUT" )
        DEFINES += KVS_SUPPORT_GLUT
        INCLUDEPATH += $$KVS_GLUT_DIR/include
    }

    equals( KVS_SUPPORT_QT, "1" ){      # すべてのプラットフォームで必要
        message( "KVS_SUPPORT_QT" )
        DEFINES += KVS_SUPPORT_QT
    }

    equals( KVS_SUPPORT_OPENXR, "1" ){  # Windows専用（オプション機能）
        message( "KVS_SUPPORT_OPENXR" )
        isEmpty( KVS_IMGUI_DIR ){
            error( "To use the OPENXR feature, you must configure IMGUI." )
        }
        else{
            win32 {
                INCLUDEPATH += $$KVS_OPENXR_DIR/include
                INCLUDEPATH += $$KVS_IMGUI_DIR/include
                DEFINES += KVS_SUPPORT_OPENXR
                # DEFINES += OPENXR_SCREEN # オプション
            }
        }
    }

    isEmpty( KVS_ASSIMP_DIR ){          # 任意（全プラットフォーム対応）
        message( "KVS_ASSIMP_DIR is not defined." )
    }
    else{
        message( "KVS_ASSIMP_DIR is set to: $$KVS_ASSIMP_DIR" )
        INCLUDEPATH += $$KVS_ASSIMP_DIR/include
        DEFINES += ASSIMP
    }
}

INCLUDEPATH += $$PWD/Widgets/
INCLUDEPATH += $$PWD/FunctionParser/
INCLUDEPATH += $$PWD/Utils/
INCLUDEPATH += $$PWD/ExtendedKVS/
INCLUDEPATH += $$PWD/ExtendedQT/

# Git ブランチ名を取得して変数に設定
GIT_BRANCH_NAME = $$system(git rev-parse --abbrev-ref HEAD)

# 定数として定義（C++で使う用）
DEFINES += GIT_BRANCH_NAME=\\\"$$GIT_BRANCH_NAME\\\"

# ログ出力（確認用）
message(Git Branch: $$GIT_BRANCH_NAME)
