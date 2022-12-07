#*****************************************************************************
#  $Id: Makefile 621 2010-09-30 08:04:55Z naohisa.sakamoto $
#*****************************************************************************

#=============================================================================
#  Include.
#=============================================================================
include ../../kvs.conf
include ../../Makefile.def


#=============================================================================
#  INCLUDE_PATH, LIBRARY_PATH, LINK_LIBRARY, INSTALL_DIR.
#=============================================================================
INCLUDE_PATH := -I../../Source
LIBRARY_PATH := -L../../Source/Core/$(OUTDIR)
LINK_LIBRARY := -lkvsCore
INSTALL_DIR  := $(KVS_DIR)


#=============================================================================
#  Include path.
#=============================================================================
ifeq "$(KVS_SUPPORT_CAVE)" "1"
INCLUDE_PATH += $(CAVE_INCLUDE_PATH)
endif

ifeq "$(KVS_SUPPORT_CUDA)" "1"
INCLUDE_PATH += $(CUDA_INCLUDE_PATH)
endif

ifeq "$(KVS_SUPPORT_SAGE)" "1"
INCLUDE_PATH += $(SAGE_INCLUDE_PATH)
endif

ifeq "$(KVS_SUPPORT_GLUT)" "1"
INCLUDE_PATH += $(GLUT_INCLUDE_PATH)
endif

ifeq "$(KVS_SUPPORT_GLEW)" "1"
INCLUDE_PATH += $(GLEW_INCLUDE_PATH)
endif

ifeq "$(KVS_SUPPORT_OPENCV)" "1"
INCLUDE_PATH += $(OPENCV_INCLUDE_PATH)
endif

INCLUDE_PATH += $(GL_INCLUDE_PATH)


#=============================================================================
#  Library path.
#=============================================================================
ifeq "$(KVS_SUPPORT_CAVE)" "1"
LIBRARY_PATH += -L../../Source/SupportCAVE/$(OUTDIR) $(CAVE_LIBRARY_PATH)
endif

ifeq "$(KVS_SUPPORT_CUDA)" "1"
LIBRARY_PATH += -L../../Source/SupportCUDA/$(OUTDIR) $(CUDA_LIBRARY_PATH)
endif

ifeq "$(KVS_SUPPORT_SAGE)" "1"
LIBRARY_PATH += -L../../Source/SupportSAGE/$(OUTDIR) $(SAGE_LIBRARY_PATH)
endif

ifeq "$(KVS_SUPPORT_GLUT)" "1"
LIBRARY_PATH += -L../../Source/SupportGLUT/$(OUTDIR) $(GLUT_LIBRARY_PATH)
endif

ifeq "$(KVS_SUPPORT_GLEW)" "1"
LIBRARY_PATH += -L../../Source/SupportGLEW/$(OUTDIR) $(GLEW_LIBRARY_PATH)
endif

ifeq "$(KVS_SUPPORT_OPENCV)" "1"
LIBRARY_PATH += -L../../Source/SupportOpenCV/$(OUTDIR) $(OPENCV_LIBRARY_PATH)
endif

LIBRARY_PATH += $(GL_LIBRARY_PATH)


#=============================================================================
#  Link library.
#=============================================================================
ifeq "$(KVS_SUPPORT_CAVE)" "1"
LINK_LIBRARY += -lkvsSupportCAVE $(CAVE_LINK_LIBRARY)
endif

ifeq "$(KVS_SUPPORT_CUDA)" "1"
LINK_LIBRARY += $(CUDA_LINK_LIBRARY)
endif

ifeq "$(KVS_SUPPORT_SAGE)" "1"
LINK_LIBRARY += -lkvsSupportSAGE $(SAGE_LINK_LIBRARY)
endif

ifeq "$(KVS_SUPPORT_GLUT)" "1"
LINK_LIBRARY += -lkvsSupportGLUT $(GLUT_LINK_LIBRARY)
endif

ifeq "$(KVS_SUPPORT_GLEW)" "1"
LINK_LIBRARY += -lkvsSupportGLEW $(GLEW_LINK_LIBRARY)
endif

ifeq "$(KVS_SUPPORT_OPENCV)" "1"
LINK_LIBRARY += -lkvsSupportOpenCV $(OPENCV_LINK_LIBRARY)
endif

LINK_LIBRARY += $(GL_LINK_LIBRARY) -lkvsCore


#=============================================================================
#  Project name.
#=============================================================================
PROJECT_NAME := kvsview

ifeq "$(findstring CYGWIN,$(shell uname -s))" "CYGWIN"
TARGET_EXE := $(OUTDIR)/$(PROJECT_NAME).exe
else
TARGET_EXE := $(OUTDIR)/$(PROJECT_NAME)
endif


#=============================================================================
#  Object.
#=============================================================================
OBJECTS := \
$(OUTDIR)/Argument.o \
$(OUTDIR)/Default.o \
$(OUTDIR)/Isosurface.o \
$(OUTDIR)/SlicePlane.o \
$(OUTDIR)/OrthoSlice.o \
$(OUTDIR)/PointRenderer.o \
$(OUTDIR)/LineRenderer.o \
$(OUTDIR)/PolygonRenderer.o \
$(OUTDIR)/ObjectInformation.o \
$(OUTDIR)/TransferFunction.o \
$(OUTDIR)/ExtractEdges.o \
$(OUTDIR)/ExtractVertices.o \
$(OUTDIR)/ExternalFaces.o \
$(OUTDIR)/RayCastingRenderer.o \
$(OUTDIR)/ParticleVolumeRenderer.o \
$(OUTDIR)/Histogram.o \
$(OUTDIR)/Widget.o \
$(OUTDIR)/main.o \


#=============================================================================
#  Build rule.
#=============================================================================
$(TARGET_EXE): $(OBJECTS)
	$(LD) $(LDFLAGS) $(LIBRARY_PATH) -o $@ $^ $(LINK_LIBRARY)

$(OUTDIR)/%.o: %.cpp %.h
	$(MKDIR) $(OUTDIR)
	$(CPP) -c $(CPPFLAGS) $(DEFINITIONS) $(INCLUDE_PATH) -o $@ $<

$(OUTDIR)/%.o: %.cpp
	$(MKDIR) $(OUTDIR)
	$(CPP) -c $(CPPFLAGS) $(DEFINITIONS) $(INCLUDE_PATH) -o $@ $<


#=============================================================================
#  build.
#=============================================================================
build: $(TARGET_EXE)


#=============================================================================
#  clean.
#=============================================================================
clean:
	$(RMDIR) $(OUTDIR)


#=============================================================================
#  install.
#=============================================================================
install:
	$(MKDIR) $(INSTALL_DIR)/bin
	$(INSTALL_EXE) $(TARGET_EXE) $(INSTALL_DIR)/bin
