include ../pbvr.conf
include ../arch/${PBVR_MACHINE}
include Depend.mk
#CXXFLAGS+=-std=c++11

ifeq "$(PBVR_SUPPORT_VTK)" "1"
    CXXFLAGS+=-I $(VTK_INCLUDE_PATH)
    export CPP := ${CXX}
    export CPPFLAGS := ${CXXFLAGS}
    ifdef MPICXX
        export CPP := ${MPICXX}
        export CPPFLAGS := ${MPICXXFLAGS}
    endif
endif

ifdef MPICXX
    CXX     = ${MPICXX}
endif

ifdef MPICCFLAGS
    CCFLAGS := ${MPICCFLAGS} -DMPI_EFFECT -DMPI_MEM_SAVE 
endif
ifdef MPICXXFLAGS
    CXXFLAGS += ${MPICXXFLAGS} -DMPI_EFFECT -DMPI_MEM_SAVE 
endif

ifeq "$(PBVR_SUPPORT_VTK)" "1"
    CXXFLAGS += -DVTK_ENABLED 
endif

DEBUG_LEVEL?=0

TARGET	:=lib/libpbvrFilter.a
INC_DIR	:=./include
DOPT	:=-DBLOCK_READ -DDEBUG_LEVEL=$(DEBUG_LEVEL)
BOPT	:=
WOPT	:=-Wall -Wno-unused -Wno-unused-result 
CXXFLAGS	+=$(DOPT) $(BOPT) $(WOPT) -I $(INC_DIR) 

IO_STL_SRC  :=$(wildcard src/filter_io_stl/*.cpp)
IO_P3D_SRC  :=$(wildcard src/filter_io_plot3d/*.cpp)
IO_AVS_SRC  :=$(wildcard src/filter_io_avs/*.cpp)
IO_VTK_SRC  :=$(wildcard src/filter_io_vtk/*.cpp)
IO_VTK_SRC  +=$(wildcard src/filter_io_vtk/wrapper/*.cpp)
MAIN_SRC    :=$(wildcard src/*.cpp)
	
ifeq "$(PBVR_SUPPORT_VTK)" "1"
    SRC :=$(IO_AVS_SRC) $(IO_STL_SRC) $(IO_P3D_SRC) $(IO_VTK_SRC) $(MAIN_SRC) 
else
    SRC :=$(IO_AVS_SRC) $(IO_STL_SRC) $(IO_P3D_SRC) $(MAIN_SRC) 
endif

OBJ :=$(SRC:.cpp=.o)

all: setup $(TARGET)

setup:
	mkdir -p lib

$(TARGET):$(OBJ)
	@echo Linking:   $(TARGET)
	$(AR) $(ARFLAGS) $(TARGET) $(OBJ)

.cpp.o:
	@echo Compiling: $< 
	$(CXX) $(CXXFLAGS) -c $<  -o $@

clean:
	rm -f $(OBJ)

all_clean:
	rm -f $(TARGET) $(OBJ)

#depend

