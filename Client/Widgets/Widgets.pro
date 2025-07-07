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
    Connect.h \
    # GlyphItem.h \
    GlyphEditor.h \
    MergeWorker.h \
    ObjectEditor.h \
    # ObjectItem.h \
    OpacityMapEditor.h \
    PlayBackControlToolBar.h \
    PlotOverLine.h \
    # PlotOverLineItem.h \
    PointSizeControl.h \
    Preference.h \
    RepetitionLevelControl.h \
    ShadingControl.h \
    TimeStepControlToolBar.h \
    TotalParticlesToolBar.h \
    TransferFunctionEditor.h \
    # TransferFunctionItem.h \
    VariableEditor.h \
    VolumeTransform.h


#=============================================================================
#  Source.
#=============================================================================
SOURCES += \
    AnimationControl.cpp \
    ColorMapEditor.cpp \
    ColorMapSelectorToolBar.cpp \
    Connect.cpp \
    GlyphEditor.cpp \
    ObjectEditor.cpp \
    OpacityMapEditor.cpp \
    PlayBackControlToolBar.cpp \
    PlotOverLine.cpp \
    PointSizeControl.cpp \
    Preference.cpp \
    RepetitionLevelControl.cpp \
    ShadingControl.cpp \
    TimeStepControlToolBar.cpp \
    TotalParticlesToolBar.cpp \
    TransferFunctionEditor.cpp \
    VariableEditor.cpp \
    VolumeTransform.cpp


#=============================================================================
#  Forms.
#=============================================================================
FORMS += \
    AnimationControl.ui \
    ColorMapEditor.ui \
    Connect.ui \
    GlyphEditor.ui \
    ObjectEditor.ui \
    OpacityMapEditor.ui \
    PlotOverLine.ui \
    PointSizeControl.ui \
    Preference.ui \
    RepetitionLevelControl.ui \
    ShadingControl.ui \
    TransferFunctionEditor.ui \
    VariableEditor.ui \
    VolumeTransform.ui


#=============================================================================
#  Resource.
#=============================================================================
RESOURCES += \
