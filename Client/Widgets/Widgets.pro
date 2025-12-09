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
    GlyphEditorWIP.h \
    ObjectEditorWIP.h \
    OpacityMapEditor.h \
    PlayBackControlToolBar.h \
    PlayBackControlToolBarWIP.h \
    PlotOverLineEditor.h \
    PlotOverLineEditorWIP.h \
    PointSizeControl.h \
    Preference.h \
    RemoteFileDialog.h \
    RepetitionLevelControl.h \
    ShadingControl.h \
    TimeStepControlToolBar.h \
    TotalParticlesToolBar.h \
    TransferFunctionEditor.h \
    TransferFunctionEditorWIP.h \
    VariableEditor.h \
    VariableEditorWIP.h \
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
    GlyphEditorWIP.cpp \
    ObjectEditorWIP.cpp \
    OpacityMapEditor.cpp \
    PlayBackControlToolBar.cpp \
    PlayBackControlToolBarWIP.cpp \
    PlotOverLineEditor.cpp \
    PlotOverLineEditorWIP.cpp \
    PointSizeControl.cpp \
    Preference.cpp \
    RepetitionLevelControl.cpp \
    ShadingControl.cpp \
    TimeStepControlToolBar.cpp \
    TotalParticlesToolBar.cpp \
    TransferFunctionEditor.cpp \
    TransferFunctionEditorWIP.cpp \
    VariableEditor.cpp \
    VariableEditorWIP.cpp \
    VolumeTransform.cpp


#=============================================================================
#  Forms.
#=============================================================================
FORMS += \
    AnimationControl.ui \
    ColorMapEditor.ui \
    Communication.ui \
    GlyphEditor.ui \
    GlyphEditorWIP.ui \
    ObjectEditorWIP.ui \
    OpacityMapEditor.ui \
    PlotOverLineEditor.ui \
    PlotOverLineEditorWIP.ui \
    PointSizeControl.ui \
    Preference.ui \
    RepetitionLevelControl.ui \
    ShadingControl.ui \
    TransferFunctionEditor.ui \
    TransferFunctionEditorWIP.ui \
    VariableEditor.ui \
    VariableEditorWIP.ui \
    VolumeTransform.ui


#=============================================================================
#  Resource.
#=============================================================================
RESOURCES += \
