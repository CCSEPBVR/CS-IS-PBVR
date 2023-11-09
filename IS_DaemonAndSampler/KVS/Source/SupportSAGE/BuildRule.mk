#  DON'T EDIT THIS FILE.
#  THIS IS GENERATED BY "Configuration/configure_BuildRule.py".


OBJECTS := \
$(OUTDIR)/./Viewer/Application.o \
$(OUTDIR)/./Viewer/ApplicationInterface.o \
$(OUTDIR)/./Viewer/GlobalBase.o \
$(OUTDIR)/./Viewer/Screen.o \
$(OUTDIR)/./Viewer/ScreenBase.o \



$(OUTDIR)/./Viewer/%.o: ./Viewer/%.cpp ./Viewer/%.h
	$(MKDIR) $(OUTDIR)/./Viewer
	$(CPP) -c $(CPPFLAGS) $(DEFINITIONS) $(INCLUDE_PATH) -o $@ $<


install::
	$(MKDIR) $(INSTALL_DIR)/include/SupportSAGE/.
	$(INSTALL) ./*.h $(INSTALL_DIR)/include/SupportSAGE/.
	$(MKDIR) $(INSTALL_DIR)/include/SupportSAGE/./Viewer
	$(INSTALL) ./Viewer/*.h $(INSTALL_DIR)/include/SupportSAGE/./Viewer
