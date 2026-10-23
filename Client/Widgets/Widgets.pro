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
    MCPWidget.h \
    ../MCP/MCPFileChannel.h \
    ../MCP/MCPProtocol.h \
    ../MCP/MCPState.h \
    ../MCP/MCPWorker.h \
    AnimationControl.h \
    ColorMapEditor.h \
    ColorMapSelectorToolBar.h \
    Communication.h \
    NetcdfAuxiliaryFileDialog.h \
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
    TransferFunctionEditor.h \
    VariableEditor.h \
    VizMode.h \
    VolumeTransform.h


#=============================================================================
#  Source.
#=============================================================================
SOURCES += \
    MCPWidget.cpp \
    ../MCP/MCPFileChannel.cpp \
    ../MCP/MCPProtocol.cpp \
    ../MCP/MCPState.cpp \
    ../MCP/MCPWorker.cpp \
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
    TransferFunctionEditor.cpp \
    VariableEditor.cpp \
    VolumeTransform.cpp


#=============================================================================
#  Forms.
#=============================================================================
FORMS += \
    MCPWidget.ui \
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
    TransferFunctionEditor.ui \
    VariableEditor.ui \
    VolumeTransform.ui


#=============================================================================
#  Resource.
#=============================================================================
RESOURCES += \
