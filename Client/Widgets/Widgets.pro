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
    Connect.h \
    GlyphEditor.h \
    ObjectEditorWIP.h \
    OpacityMapEditor.h \
    PlayBackControlToolBar.h \
    PlotOverLineEditor.h \
    PointSizeControl.h \
    Preference.h \
    RemoteFileDialog.h \
    RepetitionLevelControl.h \
    ShadingControl.h \
    TimeStepControlToolBar.h \
    TotalParticlesToolBar.h \
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
    Connect.cpp \
    GlyphEditor.cpp \
    ObjectEditorWIP.cpp \
    OpacityMapEditor.cpp \
    PlayBackControlToolBar.cpp \
    PlotOverLineEditor.cpp \
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
    Communication.ui \
    Connect.ui \
    GlyphEditor.ui \
    ObjectEditorWIP.ui \
    OpacityMapEditor.ui \
    PlotOverLineEditor.ui \
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
