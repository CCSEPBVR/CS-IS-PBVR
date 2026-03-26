include ./Depend.mk
#CPPFLAGS=$(CFLAGS) -c
ARFLAGS= 

LIB=libfilter_io_vtk.a
LIB_OBJ=wrapper/wrapper_vtk_reader.o wrapper/wrapper_vtk_data.o

.SUFFIXES: .cpp .o
.cpp.o:
	@echo Building $@
	@$(CPP) $(CPPFLAGS) -DVTK_IN_VTK -I $(VTK_INCLUDE_PATH) -c $< -o $@

$(LIB): info $(LIB_OBJ)
	@rm -f $(LIB)
	ar $(ARFLAGS) cr $(LIB) $(LIB_OBJ)
		
info:
	@echo Linking $(LIB_OBJ)
	@echo VTK_VERSION=$(VTK_VERSION)  
	@echo VTK_INCLUDE_PATH=$(VTK_INCLUDE_PATH) 	

lib_clean:
	@echo Cleaning lib
	@rm -f *.o  $(LIB) $(LIB_OBJ)

