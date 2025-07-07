include(../SETTINGS.pri)
#=============================================================================
#  Configuration valiable.
#=============================================================================

#=============================================================================
#  Template.
#=============================================================================
TEMPLATE = lib
CONFIG += static

#=============================================================================
#  Target.
#=============================================================================
TARGET = ExtendedKVS

#=============================================================================
#  Header.
#=============================================================================
HEADERS += \
    Screen.h \   
    Utils.h \
    stb_image.h

contains( DEFINES, OPENXR_SCREEN ){
HEADERS += \
    EventTimer.h \
    OpenXRInteractor.h \
    OpenXRScreen.h \
    VRHandControllerListener.h
}

contains( DEFINES, ASSIMP ){
HEADERS += \
    FBX.h \
    ThreeDS.h
}

contains( DEFINES, ASSIMP ) {
    HEADERS += TexturedPolygonImporter.h
}


#=============================================================================
#  Source.
#=============================================================================
SOURCES += \
    Screen.cpp \    
    Utils.cpp \

contains( DEFINES, OPENXR_SCREEN ){
SOURCES += \
    EventTimer.cpp \
    OpenXRInteractor.cpp \
    OpenXRScreen.cpp \
    VRHandControllerListener.cpp
}

contains( DEFINES, ASSIMP ){
SOURCES += \
    FBX.cpp \
    ThreeDS.cpp
}

contains( DEFINES, ASSIMP ) {
    SOURCES += TexturedPolygonImporter.cpp
}


#=============================================================================
#  Forms.
#=============================================================================
FORMS += \

#=============================================================================
#  Resource.
#=============================================================================
RESOURCES += \
