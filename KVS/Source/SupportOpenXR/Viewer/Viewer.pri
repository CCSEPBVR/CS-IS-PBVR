#*****************************************************************************
#  $Id: Viewer.pri Keisuke Tajiri $
#*****************************************************************************

#=============================================================================
#  sources and headers
#=============================================================================
VIEWER_HEADERS += \
Viewer/Screen.h \

VIEWER_SOURCES += \
Viewer/Screen.cpp \

HEADERS += $$VIEWER_HEADERS
SOURCES += $$VIEWER_SOURCES

debug {
	LIBS += -L$$PWD/../SupportQt/Debug -lkvsSupportQt.lib
}
release {
	LIBS += -L$$PWD/../SupportQt/Release -lkvsSupportQt.lib
}

#=============================================================================
#  install
#=============================================================================
VIEWER_INSTALL_HEADERS.files = $$VIEWER_HEADERS
VIEWER_INSTALL_HEADERS.path  = $$KVS_DIR/include/SupportOpenXR/Viewer

INSTALLS += VIEWER_INSTALL_HEADERS
