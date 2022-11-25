#*****************************************************************************
#  $Id: SupportQt.pri 275 2009-07-12 13:05:24Z naohisa.sakamoto $
#*****************************************************************************

#=============================================================================
#  KVS directory
#=============================================================================
KVS_DIR = $$(KVS_DIR)

isEmpty( KVS_DIR ) {
    error( "KVS_DIR is not defined." )
}


#=============================================================================
#  template
#=============================================================================
TEMPLATE = lib


#=============================================================================
#  target
#=============================================================================
win32 {
    TARGET = kvsSupportQt
} else {
    TARGET = kvsSupportQt
}


#=============================================================================
#  configuration valiable
#=============================================================================
CONFIG += warn_off opengl staticlib
QT     += opengl


#=============================================================================
#  definition
#=============================================================================
win32 {
    DEFINES += WIN32 _MBCS NOMINMAX _SCL_SECURE_NO_DEPRECATE _CRT_SECURE_NO_DEPRECATE _CRT_NONSTDC_NO_DEPRECATE
}

cygwin* {
    DEFINES += NOMINMAX
}

DEFINES += KVS_SUPPORT_GLUT KVS_SUPPORT_QT


#=============================================================================
#  include path
#=============================================================================
INCLUDEPATH += ..


#=============================================================================
#  install
#=============================================================================
target.path = $$KVS_DIR/lib

INSTALL_HEADERS.files = Qt.h
INSTALL_HEADERS.path  = $$KVS_DIR/include/SupportQt

INSTALLS += target INSTALL_HEADERS


#=============================================================================
#  header and source
#=============================================================================
include(Viewer/Viewer.pri)
