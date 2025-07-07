#*****************************************************************************
#  $Id: Release.pro tajiri@fiatlux.co.jp $
#*****************************************************************************

CONFIG  += release
DEFINES += NDEBUG

MOC_DIR     = Release
OBJECTS_DIR = Release
DESTDIR     = Release

KVS_CPP = $$(KVS_CPP)
!isEmpty( KVS_CPP ) {
QMAKE_CXX = $(KVS_CPP)
}

include( SupportOpenXR.pri )
