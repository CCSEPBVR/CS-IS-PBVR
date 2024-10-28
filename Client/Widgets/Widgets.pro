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
    DisplayPointSizeControl.h \
    ExtendedTransferFunctionMessage.h \
    FrequencyTable.h \
    MergePanel.h \
    OpacityMapEditor.h \
    ParameterFile.h \
    PlotOverLine.h \
    Preference.h \
    ReceivedMessage.h \
    RenderOptions.h \
    RepetitionLevelControl.h \
    ShadingController.h \
    TFEColorMapBar.h \
    TFEColorMapPalette.h \
    TFEHistogramBar.h \
    TFEOpacityMapBar.h \
    TFEOpacityMapPalette.h \
    ToolBars.h \
    TransferFunctionEditor.h \
    TransferFunctionParameter.h \
    VariableEditor.h \
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
    DisplayPointSizeControl.cpp \
    ExtendedTransferFunctionMessage.cpp \
    MergePanel.cpp \
    OpacityMapEditor.cpp \
    ParameterFile.cpp \
    PlotOverLine.cpp \
    Preference.cpp \
    RenderOptions.cpp \
    RepetitionLevelControl.cpp \
    ShadingController.cpp \
    TFEColorMapBar.cpp \
    TFEColorMapPalette.cpp \
    TFEHistogramBar.cpp \
    TFEOpacityMapBar.cpp \
    TFEOpacityMapPalette.cpp \
    ToolBars.cpp \
    TransferFunctionEditor.cpp \
    VariableEditor.cpp \
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
    DisplayPointSizeControl.ui \
    MergePanel.ui \
    OpacityMapEditor.ui \
    PlotOverLine.ui \
    Preference.ui \
    RenderOptions.ui \
    RepetitionLevelControl.ui \
    ShadingController.ui \
    TransferFunctionEditor.ui \
    VariableEditor.ui \
    VolumeTransform.ui \
    Connect.ui



#=============================================================================
#  Resource.
#=============================================================================
RESOURCES += \
