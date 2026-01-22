
VTK_VERSION?=6.0
VTK_INCLUDE_PATH?=/usr/include/vtk-$(VTK_VERSION)
VTK_LIB_PATH?=/usr/local/lib

VTK_ZLIB= $(wildcard $(VTK_LIB_PATH)/libvtkzlib-$(VTK_VERSION)*)

IO_VTK_SRC= $(wildcard filter_io_vtk/*.c)

#Check if VTK has internal zlib build
VTK_ZLIB= $(wildcard $(VTK_LIB_PATH)/libvtkzlib-$(VTK_VERSION)*)
ifneq ("$(VTK_ZLIB)","")
	LD_VTK_ZLIB+= -lvtkzlib-$(VTK_VERSION)
endif

LD_VTK_LIB+=	-lvtkCommonExecutionModel-$(VTK_VERSION) \
		-lvtkCommonDataModel-$(VTK_VERSION) \
		-lvtkCommonMisc-$(VTK_VERSION)  \
		-lvtkCommonSystem-$(VTK_VERSION) \
		-lvtksys-$(VTK_VERSION)  -lvtkCommonTransforms-$(VTK_VERSION) \
		-lvtkCommonMath-$(VTK_VERSION)      \
		-lvtkCommonCore-$(VTK_VERSION) \
		$(LD_VTK_ZLIB) \
		-lvtkIOCore-$(VTK_VERSION) \
		-lvtkIOLegacy-$(VTK_VERSION)

LD_VTK_FLAGS = -L./filter_io_vtk -L$(VTK_LIB_PATH) -lfilter_io_vtk  $(LD_VTK_LIB)
