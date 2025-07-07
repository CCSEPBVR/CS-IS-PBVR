#*****************************************************************************
#  $Id: Debug.pro tajiri@fiatlux.co.jp $
#*****************************************************************************

CONFIG  += debug
DEFINES += _DEBUG KVS_ENABLE_DEBUG

MOC_DIR     = Debug
OBJECTS_DIR = Debug
DESTDIR     = Debug

KVS_CPP = $$(KVS_CPP)
!isEmpty( KVS_CPP ) {
QMAKE_CXX = $(KVS_CPP)
}

include( SupportOpenXR.pri )
