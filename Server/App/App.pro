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
TARGET = pbvr_server

#=============================================================================
#  Link library.
#=============================================================================
win32 {
    CONFIG(release, debug|release){
        # LIBS += -L../Hoge/release -lHoge
    }
    else:CONFIG(debug, debug|release){
        # LIBS += -L../Hoge/debug -lHoge
    }

    !isEmpty( KVS_DIR ) {
        LIBS += -L$$KVS_DIR/lib -lkvsCore
        equals( KVS_ENABLE_OPENGL, "1" ) {
            LIBS += -lopengl32
        }
        equals( KVS_ENABLE_GLU, "1" ) {
            LIBS += -lglu32
        }
        equals( KVS_ENABLE_GLEW, "1" ) {
            LIBS += -L$$KVS_GLEW_DIR/lib -lglew32
        }
    }
    !isEmpty(KVS_UWS_DIR) {
        LIBS +="$$KVS_UWS_DIR/lib/uSockets.lib"
        LIBS +="$$KVS_UWS_DIR/lib/libuv.lib"
        LIBS +="$$KVS_UWS_DIR/lib/zlib.lib"

        LIBS += -ladvapi32 \
            -luserenv \
            -lkernel32 \
            -luser32 \
            -lgdi32 \
            -lws2_32 \
            -liphlpapi \
            -lsecur32 \
            -lpsapi \
            -lshell32 \
            -lole32 \
            -loleaut32 \
            -lshlwapi \
            -lDbghelp \
            -lrpcrt4
    }

}

macx {
    !isEmpty( KVS_DIR ) {
        LIBS += -L$$KVS_DIR/lib -lkvsCore
    }
    !isEmpty( KVS_UWS_DIR ) {
        LIBS += $$KVS_UWS_DIR/uSockets/uSockets.a
        OPENSSL_PATH = /opt/homebrew/opt/openssl@3
        LIBS += -L$$OPENSSL_PATH/lib -lssl -lcrypto -lz
    }
}

unix:!macx {
    !isEmpty( KVS_DIR ) {
        LIBS += -L$$KVS_DIR/lib -lkvsCore
    }
    !isEmpty( KVS_UWS_DIR ) {
        LIBS += $$KVS_UWS_DIR/uSockets/uSockets.a
        LIBS += -lssl -lcrypto -lz
    }
}

#=============================================================================
#  Header.
#=============================================================================
HEADERS += \
    Server.h

#=============================================================================
#  Source.
#=============================================================================
SOURCES += \
    Server.cpp \
    main.cpp

#=============================================================================
#  Forms.
#=============================================================================
FORMS += \

#=============================================================================
#  Resource.
#=============================================================================
RESOURCES += \
