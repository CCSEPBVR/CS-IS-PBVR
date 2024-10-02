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
    Screen.h

contains(DEFINES, PBVR_SUPPORT_FBX) | contains(DEFINES, PBVR_SUPPORT_3DS){
HEADERS += \
    TexturedPolygonImporter.h \
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
    Screen.cpp

contains(DEFINES, PBVR_SUPPORT_FBX) | contains(DEFINES, PBVR_SUPPORT_3DS){
SOURCES += \
    TexturedPolygonImporter.cpp

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
