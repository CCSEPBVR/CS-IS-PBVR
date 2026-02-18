PBVR_DIR = ../../../../

include ${PBVR_DIR}/pbvr.conf
include ${PBVR_DIR}/arch/${PBVR_MACHINE}

INSITU_DIR           = ${PBVR_DIR}/InSituLib/unstruct
INSITU_COMMON_DIR    = ${PBVR_DIR}/InSituLib/shared
VISMODULE_SOURCE_DIR = ${PBVR_DIR}/VisModule
FUNC_DIR             = ${PBVR_DIR}/FunctionParser

TARGET  = run

ifdef MPICXX
    CXX      = ${MPICXX}
endif

ifdef MPICXXFLAGS
    CXXFLAGS = ${MPICXXFLAGS}
else
	CXXFLAGS += -DCPU_VER
endif

ifdef KVS_DIR
include $(KVS_DIR)/kvs.conf
ifeq "$(KVS_SUPPORT_EXTENDED_FILE_FORMAT)" "1"
CXXFLAGS += -DEXTEND_FILE_FORMAT
endif
endif

CXXFLAGS += -I./ \
            -I../../../../Utils \
            -I../../../../App \
            -I${INSITU_DIR} \
            -I${INSITU_COMMON_DIR} \
            -I${VISMODULE_SOURCE_DIR} \
            -I${FUNC_DIR} \
            -I${KVS_DIR}/include

ifdef KVS_DIR
include $(KVS_DIR)/kvs.conf
ifeq "$(KVS_SUPPORT_EXTENDED_FILE_FORMAT)" "1"
CXXFLAGS += -I$(VTK_INCLUDE_PATH)
endif
endif

ifdef MPICXX
    LD = ${MPILD}
endif

ifdef MPILDFLAGS
    LDFLAGS += ${MPILDFLAGS}
endif

LDFLAGS  += -L${INSITU_DIR} -lInSituPBVR \
            -L${VISMODULE_SOURCE_DIR}/Release -lvisModule \
            -L${FUNC_DIR} -lpbvrFunc \
            -L${KVS_DIR}/lib -lkvsCore

ifdef KVS_DIR
include $(KVS_DIR)/kvs.conf
ifeq "$(KVS_SUPPORT_EXTENDED_FILE_FORMAT)" "1"
LDFLAGS += -L$(VTK_LIB_PATH) \
-lkvsSupportExtendedFileFormat \
-lvtkCommonExecutionModel-$(VTK_VERSION) \
-lvtkFiltersCore-$(VTK_VERSION) \
-lvtkFiltersExtraction-$(VTK_VERSION) \
-lvtkFiltersGeneral-$(VTK_VERSION) \
-lvtkFiltersGeometry-$(VTK_VERSION) \
-lvtkFiltersParallel-$(VTK_VERSION) \
-lvtkCommonExecutionModel-$(VTK_VERSION) \
-lvtkFiltersCore-$(VTK_VERSION) \
-lvtkIOCore-$(VTK_VERSION) \
-lvtkIOGeometry-$(VTK_VERSION) \
-lvtkIOCGNSReader-$(VTK_VERSION) \
-lvtkIOEnSight-$(VTK_VERSION) \
-lvtkIOParallel-$(VTK_VERSION) \
-lvtkIOLegacy-$(VTK_VERSION) \
-lvtkIOXML-$(VTK_VERSION) \
-lvtkCommonDataModel-$(VTK_VERSION) \
-lvtkIOXMLParser-$(VTK_VERSION) \
-lvtkexpat-$(VTK_VERSION) \
-lvtkCommonTransforms-$(VTK_VERSION) \
-lvtkCommonMath-$(VTK_VERSION) \
-lvtkCommonCore-$(VTK_VERSION) \
-lvtksys-$(VTK_VERSION) \
-lvtkloguru-$(VTK_VERSION) \
-lvtkIOCore-$(VTK_VERSION) \
-lvtkzlib-$(VTK_VERSION) \
-lvtkdoubleconversion-$(VTK_VERSION) \
-lvtklzma-$(VTK_VERSION) \
-lvtklz4-$(VTK_VERSION) \
-lvtkCommonSystem-$(VTK_VERSION) \
-lvtkCommonMisc-$(VTK_VERSION) \
-lvtkpugixml-$(VTK_VERSION) \
-ldl
endif
endif

#LDFLAGS += -DVTK
#LDFLAGS += -I/home/center/viztools/VTK-9.2.2/include/vtk-9.2 -I./VTK -I./Importer -I./Filter
#LDFLAGS += -L/home/center/viztools/VTK-9.2.2/lib64 -lvtkFiltersExtraction-9.2 -lvtkIOXML-9.2 -lvtkIOXMLParser-9.2 -lvtkexpat-9.2 -lvtkParallelCore-9.2 -lvtkIOLegacy-9.2 -lvtkIOCore-9.2 \
#    -lvtkdoubleconversion-9.2 -lvtklz4-9.2 -lvtklzma-9.2 -lvtkFiltersStatistics-9.2 -lvtkFiltersCore-9.2 -lvtkCommonExecutionModel-9.2  \
#    -lvtkzlib-9.2 -lvtkCommonColor-9.2 -lvtkfmt-9.2 -lvtkCommonDataModel-9.2 -lvtkpugixml-9.2 -lvtkCommonSystem-9.2 -lvtkCommonMisc-9.2 -lvtkCommonTransforms-9.2 \
#    -lvtkCommonMath-9.2 -lm -lvtkCommonCore-9.2 -lvtkloguru-9.2 -lvtksys-9.2


#nvcc -O3 -DUSE_NVCC -std=c++11 -restrict -Xcompiler -fopenmp  -lm -lstdc++ -Xcompiler -fopenmp -L/opt/mvapich2/gdr/2.3a/mcast/no-openacc/cuda8.0/mofed4.2/mpirun/gnu4.8.5/lib64  -lmpi -lmpicxx  -L/usr/local/cuda/lib64  -lcudart -rdc=true -Xnvlink -w --gpu-architecture=sm_60 -L../../../InSituLib_LBM -lInSituPBVR -L../../../FunctionParser -lpbvrFunc -L../../../KVS/Source/Core/Release -lkvsCore

TEST_SRCS = \
main.cpp \
Hydrogen.cpp

TEST_HEADERS = \
Hydrogen.h

TEST_OBJS = $(TEST_SRCS:%.cpp=%.o)

OBJS = $(TEST_OBJS)

all: $(TARGET) 

$(TARGET): $(TEST_OBJS)
	$(LD) -o $@ $^ $(LDFLAGS)

.cpp.o:
	$(CXX) $(CXXFLAGS) -o $@ -c $<

$(TEST_OBJS) : $(TEST_HEADERS)

clean:
	@rm -f *.o

all_clean:
	@rm -f *.o $(TARGET)


