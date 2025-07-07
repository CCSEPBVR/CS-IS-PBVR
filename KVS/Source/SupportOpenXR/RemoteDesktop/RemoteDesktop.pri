#*****************************************************************************
#  $Id: RemoteDesktop.pri Keisuke Tajiri $
#*****************************************************************************

#=============================================================================
#  sources and headers
#=============================================================================
REMOTEDESKTOP_HEADERS += \
RemoteDesktop/RemoteDesktop.h \

REMOTEDESKTOP_SOURCES += \
RemoteDesktop/RemoteDesktop.cpp \
$$KVS_IMGUI_DIR/include/imgui/imgui.cpp \
$$KVS_IMGUI_DIR/include/imgui/imgui_draw.cpp \
$$KVS_IMGUI_DIR/include/imgui/imgui_tables.cpp \
$$KVS_IMGUI_DIR/include/imgui/imgui_widgets.cpp \
$$KVS_IMGUI_DIR/include/imgui/imgui_impl_opengl3.cpp \

HEADERS += $$REMOTEDESKTOP_HEADERS
SOURCES += $$REMOTEDESKTOP_SOURCES

LIBS += -lgdi32

#=============================================================================
#  install
#=============================================================================
REMOTEDESKTOP_INSTALL_HEADERS.files = $$REMOTEDESKTOP_HEADERS
REMOTEDESKTOP_INSTALL_HEADERS.path  = $$KVS_DIR/include/SupportOpenXR/RemoteDesktop

INSTALLS += REMOTEDESKTOP_INSTALL_HEADERS
