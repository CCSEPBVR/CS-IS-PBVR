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
TARGET = Widgets

#=============================================================================
#  Header.
#=============================================================================
HEADERS += \
    AnimationControl.h \
    ColorMapEditor.h \
    ColorMapSelectorToolBar.h \
    Communication.h \
    GlyphEditor.h \
    ObjectEditor.h \
    OpacityMapEditor.h \
    PlayBackControlToolBar.h \
    PlotOverLineEditor.h \
    PlotOverTimeEditor.h \
    PointSizeControl.h \
    Preference.h \
    RemoteFileDialog.h \
    RepetitionLevelControl.h \
    ShadingControl.h \
    TimeStepControlToolBar.h \
    TotalParticlesToolBar.h \
    EnsembleTransferFunctionEditor.h \
    TransferFunctionEditor.h \
    VariableEditor.h \
    VizMode.h \
    VolumeTransform.h


#=============================================================================
#  Source.
#=============================================================================
SOURCES += \
    AnimationControl.cpp \
    ColorMapEditor.cpp \
    ColorMapSelectorToolBar.cpp \
    Communication.cpp \
    GlyphEditor.cpp \
    ObjectEditor.cpp \
    OpacityMapEditor.cpp \
    PlayBackControlToolBar.cpp \
    PlotOverLineEditor.cpp \
    PlotOverTimeEditor.cpp \
    PointSizeControl.cpp \
    Preference.cpp \
    RepetitionLevelControl.cpp \
    ShadingControl.cpp \
    TimeStepControlToolBar.cpp \
    TotalParticlesToolBar.cpp \
    EnsembleTransferFunctionEditor.cpp \
    TransferFunctionEditor.cpp \
    VariableEditor.cpp \
    VolumeTransform.cpp


#=============================================================================
#  Forms.
#=============================================================================
FORMS += \
    AnimationControl.ui \
    ColorMapEditor.ui \
    Communication.ui \
    GlyphEditor.ui \
    ObjectEditor.ui \
    OpacityMapEditor.ui \
    PlotOverLineEditor.ui \
    PlotOverTimeEditor.ui \
    PointSizeControl.ui \
    Preference.ui \
    RepetitionLevelControl.ui \
    ShadingControl.ui \
    EnsembleTransferFunctionEditor.ui \
    TransferFunctionEditor.ui \
    VariableEditor.ui \
    VolumeTransform.ui


#=============================================================================
#  Resource.
#=============================================================================
RESOURCES += \
