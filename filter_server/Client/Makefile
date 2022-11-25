include ../pbvr.conf
include ../arch/${PBVR_MACHINE}

TARGET           = pbvr_client
FUNC_DIR         = ../FunctionParser
COMMON_DIR       = ../Common
KVS_SOURCE_DIR   = ../KVS/Source
GLUI_DIR         = ../glui/src
CXXFLAGS        += -DCPUMODE -DNDEBUG -DKVS_SUPPORT_GLUT \
                   -I${KVS_SOURCE_DIR} \
                   -I${COMMON_DIR} \
                   -I${FUNC_DIR} \
                   -I${GLUI_DIR}/include \
                   -I./
LDFLAGS         += -L${KVS_SOURCE_DIR}/SupportGLUT/Release -lkvsSupportGLUT \
                   -L${KVS_SOURCE_DIR}/Core/Release        -lkvsCore        \
                   -L${COMMON_DIR}                         -lpbvrCommon \
                   -L${FUNC_DIR}                           -lpbvrFunc \
                   -L${GLUI_DIR}/lib                       -lglui

ifeq "$(shell uname -s)" "Darwin"
    CXXFLAGS += -std=c++11
    LDFLAGS += -framework glut -framework OpenGL
else
ifeq "$(shell uname -s)" "Linux"
    LDFLAGS += -lGLU -lGL -lglut -lm -lpthread
endif
endif

CLIENT_SRCS = ColorMapLibrary.cpp \
              Command.cpp \
              ComThread.cpp \
              Controller.cpp \
              CoordinatepanelUI.cpp \
              FileDialog.cpp \
              VizParameterFile.cpp \
              KeyFrameAnimation.cpp \
              KVSMLObjectPointWriter.cpp \
              Label.cpp \
              LegendDivisionColor.cpp \
              LegendFrameColor.cpp \
              LegendpanelUI.cpp \
              main.cpp \
              ExtendedTransferFunctionMessage.cpp \
              ParameterFile.cpp \
              ParticleMerger.cpp \
              ParticleMergerUI.cpp \
              ParticleServer.cpp \
              ProgressBar.cpp \
              sighandler.cpp \
              TransferFunctionEditorColorFreeformCurve.cpp \
              TransferFunctionEditorColorExpression.cpp \
              TransferFunctionEditorColorControlPoints.cpp \
              TransferFunctionEditorColorSelectColormap.cpp \
              TransferFunctionEditorMain.cpp \
              TransferFunctionEditorOpacityFreeformCurve.cpp \
              TransferFunctionEditorOpacityExpression.cpp \
              TransferFunctionEditorOpacityControlPoints.cpp \
              TimerEvent.cpp \
              TransferFunctionEditor.cpp \
              ViewerBackgroundColor.cpp \
              ViewerpanelUI.cpp \
              TransferFunctionEditorColorFunction.cpp

CLIENT_HEADERS = AnimationControl.h \
                 Argument.h \
                 ColorMapLibrary.h \
                 Command.h \
                 ComThread.h \
                 Controller.h \
                 CoordinatepanelUI.cpp \
                 CropParameter.h \
                 endian2.h \
                 FileDialog.h \
                 VizParameterFile.h \
                 FrequencyTable.h \
                 Histogram.h \
                 KeyFrameAnimation.h \
                 KeyPressEvent.h \
                 KVSMLObjectPointWriter.h \
                 Label.h \
                 LegendDivisionColor.h \
                 LegendFrameColor.h \
                 LegendpanelUI.h \
                 MemoryMonitor.h \
                 ExtendedTransferFunctionMessage.h \
                 ParameterFile.h \
                 ParticleMergeParameter.h \
                 ParticleMerger.h \
                 ParticleMergerUI.h \
                 ParticleServer.h \
                 VisualizationParameter.h \
                 ReceivedMessage.h \
                 ProgressBar.h \
                 ExtendedParticleVolumeRenderer.h \
                 sighandler.h \
                 TransferFunctionEditorColorFreeformCurve.h \
                 TransferFunctionEditorColorExpression.h \
                 TransferFunctionEditorColorControlPoints.h \
                 TransferFunctionEditorColorSelectColormap.h \
                 TransferFunctionEditorMain.h \
                 TransferFunctionEditorOpacityFreeformCurve.h \
                 TransferFunctionEditorOpacityExpression.h \
                 TransferFunctionEditorOpacityControlPoints.h \
                 TimerEvent.h \
                 timer_simple.h \
                 TransferFunctionEditor.h \
                 ViewerBackgroundColor.h \
                 ViewerpanelUI.h \
                 Types.h \
                 v2foot.h \
                 v2head.h \
                 v2newpanel.h \
                 v3head.h \
                 TransferFunctionEditorColorFunction.h

CLIENT_OBJS = $(CLIENT_SRCS:%.cpp=%.o)

all: $(TARGET) 

$(TARGET): $(CLIENT_OBJS)
	$(LD) -o $@ $^ $(LDFLAGS)

.cpp.o:
	$(CXX) $(CXXFLAGS) -o $@ -c $<

$(CLIENT_OBJS) : $(CLIENT_HEADERS)

clean:
	@rm -f *.o

all_clean:
	@rm -f *.o $(TARGET)

