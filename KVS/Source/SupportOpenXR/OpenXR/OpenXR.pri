#*****************************************************************************
#  $Id: OpenXR.pri Keisuke Tajiri $
#*****************************************************************************

#=============================================================================
#  sources and headers
#=============================================================================
OPENXR_HEADERS += \
OpenXR/OpenXRInteractor.h \
OpenXR/OpenXRDevice.h \

OPENXR_SOURCES += \
OpenXR/OpenXRInteractor.cpp \
OpenXR/OpenXRDevice.cpp \

HEADERS += $$OPENXR_HEADERS
SOURCES += $$OPENXR_SOURCES

LIBS += -L$$(KVS_OPENXR_DIR)/lib -lopenxr_loader.lib

#=============================================================================
#  install
#=============================================================================
OPENXR_INSTALL_HEADERS.files = $$OPENXR_HEADERS
OPENXR_INSTALL_HEADERS.path  = $$KVS_DIR/include/SupportOpenXR/OpenXR

INSTALLS += OPENXR_INSTALL_HEADERS
