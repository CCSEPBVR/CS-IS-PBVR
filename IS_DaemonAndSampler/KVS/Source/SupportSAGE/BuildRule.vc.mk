#  DON'T EDIT THIS FILE.
#  THIS IS GENERATED BY "Configuration/configure_BuildRule.py".


OBJECTS = \
$(OUTDIR)\.\Viewer\Application.obj \
$(OUTDIR)\.\Viewer\ApplicationInterface.obj \
$(OUTDIR)\.\Viewer\GlobalBase.obj \
$(OUTDIR)\.\Viewer\Screen.obj \
$(OUTDIR)\.\Viewer\ScreenBase.obj \



{.\Viewer\}.cpp{$(OUTDIR)\.\Viewer\}.obj::
	IF NOT EXIST $(OUTDIR)\.\Viewer $(MKDIR) $(OUTDIR)\.\Viewer
	$(CPP) /c $(CPPFLAGS) $(DEFINITIONS) $(INCLUDE_PATH) /Fo$(OUTDIR)\.\Viewer\ @<<
$<
<<


install::
	IF NOT EXIST $(INSTALL_DIR)\include\SupportSAGE\. $(MKDIR) $(INSTALL_DIR)\include\SupportSAGE\.
	$(INSTALL) .\*.h $(INSTALL_DIR)\include\SupportSAGE\.
	IF NOT EXIST $(INSTALL_DIR)\include\SupportSAGE\.\Viewer $(MKDIR) $(INSTALL_DIR)\include\SupportSAGE\.\Viewer
	$(INSTALL) .\Viewer\*.h $(INSTALL_DIR)\include\SupportSAGE\.\Viewer
