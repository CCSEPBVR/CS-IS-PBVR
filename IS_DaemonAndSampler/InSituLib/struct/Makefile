include ../../pbvr.conf
include ../../arch/${PBVR_MACHINE}

PROG=libInSituPBVR.a

KVS_SOURCE_DIR        = ../../KVS/Source
TRANSFER_FUNCTION_DIR = ./TFS
FUNCTION_PARSER_DIR   = ../../FunctionParser


OBJS = Serializer.o \
       TransferFunction.o \
       ObjectBase.o \
       TransferFunctionSynthesizer.o \
       TrilinearInterpolator.o \
       ParamInfo.o \
       VolumeObjectBase.o \
       kvs_wrapper.o

VPATH=${TRANSFER_FUNCTION_DIR}

ifdef MPICXX
    CXX      = ${MPICXX}
endif

ifdef MPICXXFLAGS
    CXXFLAGS = ${MPICXXFLAGS}
endif

CXXFLAGS += -I../shared
CXXFLAGS += -I${KVS_SOURCE_DIR}
CXXFLAGS += -I${FUNCTION_PARSER_DIR}
CXXFLAGS += -DNO_CLIENT

all : ${PROG}

${PROG} : ${OBJS}
	${AR} ${ARFLAGS} $@ ${OBJS}

%.o : %.c
	${CC} -c ${CCFLAGS} $<

%.o : %.cpp
	${CXX} -c ${CXXFLAGS} $<

clean :
	${RM} -f ${OBJS}  *.a *.o core *.bmp *.linkinfo *.vtk

all_clean :
	${RM} -f ${PROG} ${OBJS}  *.o core *.bmp *.linkinfo *.vtk
