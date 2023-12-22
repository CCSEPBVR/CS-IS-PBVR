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
TARGET = Widgets



#=============================================================================
#  Header.
#=============================================================================
HEADERS += \
    AnimationControls.h \
    Coordinates.h \
    DataProperties.h \
    DataSummary.h \
    Merge.h \
    Preference.h \
    RenderOptions.h \
    RepetitionLevelControl.h \
    TimeControl.h \
    VolumeTransform.h \
    Connect.h



#=============================================================================
#  Source.
#=============================================================================
SOURCES += \
    AnimationControls.cpp \
    Coordinates.cpp \
    DataProperties.cpp \
    DataSummary.cpp \
    Merge.cpp \
    Preference.cpp \
    RenderOptions.cpp \
    RepetitionLevelControl.cpp \
    TimeControl.cpp \
    VolumeTransform.cpp \
    Connect.cpp



#=============================================================================
#  Forms.
#=============================================================================
FORMS += \
    AnimationControls.ui \
    Coordinates.ui \
    DataProperties.ui \
    DataSummary.ui \
    Merge.ui \
    Preference.ui \
    RenderOptions.ui \
    RepetitionLevelControl.ui \
    TimeControl.ui \
    VolumeTransform.ui \
    Connect.ui



#=============================================================================
#  Resource.
#=============================================================================
RESOURCES += \
