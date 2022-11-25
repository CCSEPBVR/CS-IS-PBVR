#*****************************************************************************
#  $Id: Viewer.pri 329 2009-08-24 10:24:05Z naohisa.sakamoto $
#*****************************************************************************

#=============================================================================
#  sources and headers
#=============================================================================
VIEWER_HEADERS += \
Viewer/Application.h \
Viewer/Screen.h \
Viewer/Timer.h \
Viewer/Window.h \
Viewer/GlobalBase.h \
Viewer/ScreenBase.h \
Viewer/QtScreenSupport.h \
Viewer/KVSKey.h \
Viewer/KVSMouseButton.h \


VIEWER_SOURCES += \
Viewer/Application.cpp \
Viewer/Screen.cpp \
Viewer/Timer.cpp \
Viewer/Window.cpp \
Viewer/ScreenBase.cpp \
Viewer/GlobalBase.cpp \


HEADERS += $$VIEWER_HEADERS
SOURCES += $$VIEWER_SOURCES


#=============================================================================
#  install
#=============================================================================
VIEWER_INSTALL_HEADERS.files = $$VIEWER_HEADERS
VIEWER_INSTALL_HEADERS.path  = $$KVS_DIR/include/SupportQt/Viewer

INSTALLS += VIEWER_INSTALL_HEADERS
