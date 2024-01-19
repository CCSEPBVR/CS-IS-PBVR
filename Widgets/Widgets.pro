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
    ColorMapEditor.h \
    Coordinates.h \
    DataProperties.h \
    DataSummary.h \
    ExtendedTransferFunctionMessage.h \
    FrequencyTable.h \
    Merge.h \
    OpacityMapEditor.h \
    Preference.h \
    RenderOptions.h \
    RepetitionLevelControl.h \
    TFEColorMapBar.h \
    TFEColorMapPalette.h \
    TFEHistogramBar.h \
    TFEOpacityMapBar.h \
    TFEOpacityMapPalette.h \
    TimeControl.h \
    TransferFunctionEditor.h \
    VolumeTransform.h \
    Connect.h



#=============================================================================
#  Source.
#=============================================================================
SOURCES += \
    AnimationControls.cpp \
    ColorMapEditor.cpp \
    Coordinates.cpp \
    DataProperties.cpp \
    DataSummary.cpp \
    ExtendedTransferFunctionMessage.cpp \
    Merge.cpp \
    OpacityMapEditor.cpp \
    Preference.cpp \
    RenderOptions.cpp \
    RepetitionLevelControl.cpp \
    TFEColorMapBar.cpp \
    TFEColorMapPalette.cpp \
    TFEHistogramBar.cpp \
    TFEOpacityMapBar.cpp \
    TFEOpacityMapPalette.cpp \
    TimeControl.cpp \
    TransferFunctionEditor.cpp \
    VolumeTransform.cpp \
    Connect.cpp



#=============================================================================
#  Forms.
#=============================================================================
FORMS += \
    AnimationControls.ui \
    ColorMapEditor.ui \
    Coordinates.ui \
    DataProperties.ui \
    DataSummary.ui \
    Merge.ui \
    OpacityMapEditor.ui \
    Preference.ui \
    RenderOptions.ui \
    RepetitionLevelControl.ui \
    TimeControl.ui \
    TransferFunctionEditor.ui \
    VolumeTransform.ui \
    Connect.ui



#=============================================================================
#  Resource.
#=============================================================================
RESOURCES += \
