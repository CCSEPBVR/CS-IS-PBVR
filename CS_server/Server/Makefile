include ../pbvr.conf
include ../arch/${PBVR_MACHINE}

TARGET        = pbvr_server
FUNC_DIR      = ../FunctionParser
COMMON_DIR    = ../Common/
VIS_MODULE_SOURCE_DIR= ../VisModule

ifdef MPICXX
    CXX     = ${MPICXX}
endif

ifdef MPICXXFLAGS
    CXXFLAGS = ${MPICXXFLAGS}
endif

ifdef MPILD
    LD      = ${MPILD}
endif

ifdef MPILDFLAGS
    LDFLAGS = ${MPILDFLAGS}
endif

ifndef MPICXX
    CXXFLAGS += -DCPU_VER
endif

#CXXFLAGS += -w  -DDUPLICATION -DCPU_SAMPLING_TIME -DJPV_CLIENT -I${VIS_MODULE_SOURCE_DIR} -I${COMMON_DIR} -I${FUNC_DIR} -I./
#CXXFLAGS += -w -msse2 -DDUPLICATION -DCPU_SAMPLING_TIME -DJPV_CLIENT -I${VIS_MODULE_SOURCE_DIR} -I${COMMON_DIR} -I${FUNC_DIR} -I./
#CXXFLAGS += -w -qopt-report=5 -DSFMT_MEXP=607 -DCPU_SAMPLING_TIME -DJPV_CLIENT -I${VIS_MODULE_SOURCE_DIR} -I${COMMON_DIR} -I${FUNC_DIR} -I./
CXXFLAGS += -w -DCPU_SAMPLING_TIME -DJPV_CLIENT -I${VIS_MODULE_SOURCE_DIR} -I${COMMON_DIR} -I${FUNC_DIR} -I./
LDFLAGS  += -L${COMMON_DIR} -lpbvrCommon -L${FUNC_DIR} -lpbvrFunc -L${VIS_MODULE_SOURCE_DIR}/Release -lvisModule

SERVER_SRCS =  \
			  CellByCellHistogram.cpp \
              CropRegion.cpp \
              ExtendedTransferFunction.cpp \
              FilterInformation.cpp \
              AggregateTypeSubvolume.cpp \
              StepAggregateTypeSubvolume.cpp \
              JobCollector.cpp \
              JobDispatcher.cpp \
              KVSMLObjectPointWriter.cpp \
              KVSMLObjectGlyph.cpp \
              KVSMLObjectPlotOverLine.cpp \
              SPLITTypeSubvolume.cpp \
              main.cpp \
              ParameterFile.cpp \
			  PlotOverLine.cpp \
			  PlotOverLineProperty.cpp \
			  POLObjectGenerator.cpp \
              PointObject.cpp \
              PointObjectGenerator.cpp \
              PrismaticCell.cpp \
              RangeEstimater.cpp \
			  timer.cpp \
		      SFMT/SFMT.cpp\
              GlyphGenerator.cpp \
              GlyphProperty.cpp \
			  GlyphObjectGenerator.cpp \
		      TransferFunction.cpp \
              TransferFunctionSynthesizer.cpp \
              TransferFunctionSynthesizerCreator.cpp \
#              TriangleCell.cpp \
#              SquareCell.cpp \
#              QuadraticSquareCell.cpp \
#              QuadraticTriangleCell.cpp \


SERVER_HEADERS = Argument.h \
                 CellByCellHistogram.h \
                 common.h \
                 CropRegion.h \
                 ExtendedTransferFunction.h \
                 endian2.h \
                 FilterInformation.h \
				 FileChecker.h \
                 AggregateTypeSubvolume.h \
                 StepAggregateTypeSubvolume.h \
                 JobCollector.h \
                 JobDispatcher.h \
                 KVSMLObjectPointWriter.h \
                 KVSMLObjectGlyph.h \
	             KVSMLObjectPlotOverLine.h \
                 SPLITTypeSubvolume.h \
                 ParameterFile.h \
                 Platform.h \
			  	 PlotOverLine.h \
			  	 PlotOverLineProperty.h \
			  	 POLObjectGenerator.h \
                 PointObjectGenerator.h \
                 PrismaticCell.h \
                 RangeEstimater.h \
                 timer_simple.h \
				 timer.h \
		         SFMT/SFMT.h\
                 GlyphGenerator.h \
                 GlyphProperty.h \
				 GlyphObjectGenerator.h \
                 TransferFunction.h \
                 TransferFunctionSynthesizerCreator.h \
                 TransferFunctionSynthesizer.h \
                 Types.h \
#                 SquareCell.h \
#                 TriangleCell.h \
#                 QuadraticTriangleCell.h \
#                 QuadraticSquareCell.h 

SERVER_OBJS = $(SERVER_SRCS:%.cpp=%.o)
FUNC_OBJS   = $(FUNC_SRCS:%.cpp=%.o)

OBJS = $(SERVER_OBJS)

all: $(TARGET) 

$(TARGET): $(SERVER_OBJS)
	$(LD) -o $@ $^ $(LDFLAGS)

.cpp.o:
	$(CXX) $(CXXFLAGS) -o $@ -c $<

$(SERVER_OBJS) : $(SERVER_HEADERS)

clean:
	@rm -f *.o *optrpt SFMT/*o

all_clean:
	@rm -f *.o $(TARGET)


