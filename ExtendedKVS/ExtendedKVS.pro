#=============================================================================
#  Common Settings.
#=============================================================================
include(../SETTINGS.pri)


#=============================================================================
#  Template.
#=============================================================================
TEMPLATE = lib
CONFIG += static
QT += opengl

greaterThan( QT_MAJOR_VERSION, 5 ) {
QT += openglwidgets
}


#=============================================================================
#  Target.
#=============================================================================
TARGET = ExtendedKVS


#=============================================================================
#  Header.
#=============================================================================
HEADERS += \
    CustomObjectManager.h \    
    LASImporter.h \
    PTSImporter.h \
    Screen.h

contains(DEFINES, PBVR_SUPPORT_FBX) | contains(DEFINES, PBVR_SUPPORT_3DS){
HEADERS += \
    StochasticTexturedPolygonRenderer.h \
    TexturedPolygonImporter.h \
    TexturedPolygonObject.h \
    stb_image.h

    win32{
        HEADERS += \
        Utils.h
    }
}
contains(DEFINES, PBVR_SUPPORT_FBX){
HEADERS += \
    FBX.h
}
contains(DEFINES, PBVR_SUPPORT_3DS){
HEADERS += \
    ThreeDS.h
}

#=============================================================================
#  Source.
#=============================================================================
SOURCES += \
    CustomObjectManager.cpp \    
    LASImporter.cpp \
    PTSImporter.cpp \
    Screen.cpp

contains(DEFINES, PBVR_SUPPORT_FBX) | contains(DEFINES, PBVR_SUPPORT_3DS){
SOURCES += \
    StochasticTexturedPolygonRenderer.cpp \
    TexturedPolygonImporter.cpp \
    TexturedPolygonObject.cpp

    win32{
        SOURCES += \
        Utils.cpp
    }
}
contains(DEFINES, PBVR_SUPPORT_FBX){
SOURCES += \
    FBX.cpp
}
contains(DEFINES, PBVR_SUPPORT_3DS){
SOURCES += \
    ThreeDS.cpp
}

#=============================================================================
#  Forms.
#=============================================================================
FORMS += \


#=============================================================================
#  Resource.
#=============================================================================
RESOURCES += \
