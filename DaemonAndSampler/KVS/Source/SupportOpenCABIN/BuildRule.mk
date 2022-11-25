#  DON'T EDIT THIS FILE.
#  THIS IS GENERATED BY "Configuration/configure_BuildRule.py".


OBJECTS := \
$(OUTDIR)/./Viewer/Application.o \
$(OUTDIR)/./Viewer/Camera.o \
$(OUTDIR)/./Viewer/Configuration.o \
$(OUTDIR)/./Viewer/MainLoop.o \
$(OUTDIR)/./Viewer/Master.o \
$(OUTDIR)/./Viewer/Screen.o \
$(OUTDIR)/./Viewer/Trackpad.o \



$(OUTDIR)/./Viewer/%.o: ./Viewer/%.cpp ./Viewer/%.h
	$(MKDIR) $(OUTDIR)/./Viewer
	$(CPP) -c $(CPPFLAGS) $(DEFINITIONS) $(INCLUDE_PATH) -o $@ $<


install::
	$(MKDIR) $(INSTALL_DIR)/include/SupportOpenCABIN/.
	$(INSTALL) ./*.h $(INSTALL_DIR)/include/SupportOpenCABIN/.
	$(MKDIR) $(INSTALL_DIR)/include/SupportOpenCABIN/./Viewer
	$(INSTALL) ./Viewer/*.h $(INSTALL_DIR)/include/SupportOpenCABIN/./Viewer
