include ../pbvr.conf
include ../arch/${PBVR_MACHINE}
ifeq "$(PBVR_SUPPORT_VTK)" "1"
    include filter_io_vtk/Depend.mk
    export CPP := ${CXX}
    export CPPFLAGS := ${CXXFLAGS}
    ifdef MPICXX
        export CPP := ${MPICXX}
        export CPPFLAGS := ${MPICXXFLAGS}
    endif
endif

ifdef MPICC
    CC := ${MPICC}
endif


ifdef MPICCFLAGS
    CCFLAGS := ${MPICCFLAGS} -DMPI_EFFECT -DMPI_MEM_SAVE 
endif

ifdef MPILD
    LD      := ${MPILD}
endif

ifdef MPILDFLAGS
    LDFLAGS := ${MPILDFLAGS}
endif

ifeq "$(PBVR_SUPPORT_VTK)" "1"
    CCFLAGS += -DVTK_ENABLED 
    LDFLAGS += $(LD_VTK_FLAGS)
endif

DEBUG_LEVEL?=0

TARGET	:=pbvr_filter
INC_DIR	:=.
DOPT	:=-DBLOCK_READ -DDEBUG_LEVEL=$(DEBUG_LEVEL)
BOPT	:=
WOPT	:=-Wall -Wno-unused -Wno-unused-result 
CCFLAGS	+=$(DOPT) $(BOPT) $(WOPT) -I $(INC_DIR) 
LDFLAGS	+=-lm $(POPT)

IO_STL_SRC  :=$(wildcard filter_io_stl/*.c)
IO_P3D_SRC  :=$(wildcard filter_io_plot3d/*.c)
IO_AVS_SRC  :=$(wildcard filter_io_avs/*.c)
MAIN_SRC    :=$(wildcard *.c)
	
ifeq "$(PBVR_SUPPORT_VTK)" "1"
    SRC :=$(IO_AVS_SRC) $(IO_STL_SRC) $(IO_P3D_SRC) $(IO_VTK_SRC) $(MAIN_SRC) 
else
    SRC :=$(IO_AVS_SRC) $(IO_STL_SRC) $(IO_P3D_SRC) $(MAIN_SRC) 
endif

OBJ :=$(SRC:.c=.o)

ifeq "$(PBVR_SUPPORT_VTK)" "1"
    OBJ += filter_io_vtk/libfilter_io_vtk.a
endif

all:$(TARGET)

$(TARGET):$(OBJ)
	@echo Linking:   $(TARGET)
	$(LD) $(CCFLAGS) -o $(TARGET) $^ $(LDFLAGS)

.c.o:
	@echo Compiling: $< 
	$(CC) $(CCFLAGS) -c $<  -o $@

clean:
	rm -f $(OBJ)
	cd filter_io_vtk && make -f Makefile.mk lib_clean

all_clean:
	rm -f $(TARGET) $(OBJ)
	cd filter_io_vtk && make -f Makefile.mk lib_clean

filter_io_vtk/libfilter_io_vtk.a:
	@echo
	@echo ------------ Making VTK IO Library --------------
	@cd filter_io_vtk && make -f Makefile.mk libfilter_io_vtk.a

#depend
filter_main.o:filter_main.c filter.h

filter_io_avs/filter_avsbin.o:filter_io_avs/filter_avsbin.c filter.h
filter_io_avs/filter_avsfld.o:filter_io_avs/filter_avsfld.c filter.h
filter_io_avs/filter_ucdinp.o:filter_io_avs/filter_ucdinp.c filter.h
filter_io_avs/filter_io_avs.o:filter_io_avs/filter_io_avs.c filter.h

filter_io_stl/filter_io_stl.o:filter_io_stl/filter_io_stl.c filter.h

filter_io_vtk/filter_io_vtk.o:filter_io_vtk/filter_io_vtk.c filter_io_vtk/libfilter_io_vtk.a

filter_utils.o:filter_utils.c filter.h
filter_tree.o:filter_tree.c filter.h
filter_write.o:filter_write.c filter.h
filter_log.o:filter_log.c filter_log.h
