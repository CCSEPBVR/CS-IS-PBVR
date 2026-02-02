PBVR_DIR = ../../../..

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
