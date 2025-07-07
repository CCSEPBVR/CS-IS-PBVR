#*****************************************************************************
#  $Id: SupportQt.pri tajiri@fiatlux.co.jp $
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
    TARGET = kvsSupportOpenXR
} else {
    TARGET = kvsSupportOpenXR
}


#=============================================================================
#  configuration valiable
#=============================================================================
CONFIG += warn_off opengl staticlib
win32 {
    CONFIG += static_runtime
    Release:QMAKE_CXXFLAGS_RELEASE += /MT
    Release:QMAKE_LFLAGS_RELEASE += /MT
    Debug:QMAKE_CXXFLAGS_DEBUG += /MTd
    Debug:QMAKE_LFLAGS_DEBUG += /MTd
}
QT     += core

greaterThan( QT_MAJOR_VERSION, 5 ) {
QT += openglwidgets
}

#=============================================================================
#  definition
#=============================================================================
win32 {
    DEFINES += WIN32 _MBCS NOMINMAX _SCL_SECURE_NO_DEPRECATE _CRT_SECURE_NO_DEPRECATE _CRT_NONSTDC_NO_DEPRECATE
}

#DEFINES += KVS_ENABLE_DEBUG
DEFINES += KVS_ENABLE_OPENGL
DEFINES += KVS_SUPPORT_QT
DEFINES += KVS_ENABLE_GLEW
DEFINES += KVS_SUPPORT_OPENXR

#=============================================================================
#  include path
#=============================================================================
INCLUDEPATH += ..

KVS_GLEW_DIR = $$(KVS_GLEW_DIR)
!isEmpty( KVS_GLEW_DIR )
{
    INCLUDEPATH += $$KVS_GLEW_DIR/include
}

KVS_OPENXR_DIR = $$(KVS_OPENXR_DIR)
!isEmpty( KVS_OPENXR_DIR )
{
    INCLUDEPATH += $$KVS_OPENXR_DIR/include
}

KVS_IMGUI_DIR = $$(KVS_IMGUI_DIR)
!isEmpty( KVS_IMGUI_DIR )
{
    INCLUDEPATH += $$KVS_IMGUI_DIR/include
}


#=============================================================================
#  install
#=============================================================================
target.path = $$KVS_DIR/lib

INSTALL_HEADERS.files = OpenXR.h
INSTALL_HEADERS.path  = $$KVS_DIR/include/SupportOpenXR

INSTALLS += target INSTALL_HEADERS


#=============================================================================
#  header and source
#=============================================================================
include(Models/Models.pri)
include(OpenXR/OpenXR.pri)
include(RemoteDesktop/RemoteDesktop.pri)
include(Viewer/Viewer.pri)
