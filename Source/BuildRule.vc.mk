#  DON'T EDIT THIS FILE.

OBJECTS = \
$(OUTDIR)/PBVRFileInformation/UnstructuredPfi.obj \
$(OUTDIR)/Exporter/VtkExport.obj \
$(OUTDIR)/Exporter/UnstructuredVolumeObjectExporter.obj \
$(OUTDIR)/Exporter/StructuredVolumeObjectExporter.obj \
$(OUTDIR)/FileFormat/VtkCompositeDataSetFileFormat.obj \
$(OUTDIR)/Importer/VtkImport.obj \


{.\PBVRFileInformation\}.cpp{$(OUTDIR)\PBVRFileInformation\}.obj::
	IF NOT EXIST $(OUTDIR)\PBVRFileInformation $(MKDIR) $(OUTDIR)\PBVRFileInformation
	$(CPP) /c $(CPPFLAGS) $(DEFINITIONS) $(INCLUDE_PATH) /Fo$(OUTDIR)\PBVRFileInformation\ @<<
$<
<<

{.\Exporter\}.cpp{$(OUTDIR)\Exporter\}.obj::
	IF NOT EXIST $(OUTDIR)\Exporter $(MKDIR) $(OUTDIR)\Exporter
	$(CPP) /c $(CPPFLAGS) $(DEFINITIONS) $(INCLUDE_PATH) /Fo$(OUTDIR)\Exporter\ @<<
$<
<<

{.\FileFormat\}.cpp{$(OUTDIR)\FileFormat\}.obj::
	IF NOT EXIST $(OUTDIR)\FileFormat $(MKDIR) $(OUTDIR)\FileFormat
	$(CPP) /c $(CPPFLAGS) $(DEFINITIONS) $(INCLUDE_PATH) /Fo$(OUTDIR)\FileFormat\ @<<
$<
<<

{.\Importer\}.cpp{$(OUTDIR)\Importer\}.obj::
	IF NOT EXIST $(OUTDIR)\Importer $(MKDIR) $(OUTDIR)\Importer
	$(CPP) /c $(CPPFLAGS) $(DEFINITIONS) $(INCLUDE_PATH) /Fo$(OUTDIR)\Importer\ @<<
$<
<<


install::
	IF NOT EXIST $(INSTALL_DIR)\include $(MKDIR) $(INSTALL_DIR)\include
	$(INSTALL) .\*.h $(INSTALL_DIR)\include
	IF NOT EXIST $(INSTALL_DIR)\include\Exporter $(MKDIR) $(INSTALL_DIR)\include\Exporter
	$(INSTALL) .\Exporter\*.h $(INSTALL_DIR)\include\Exporter
# FileFormat
	IF NOT EXIST $(INSTALL_DIR)\include\FileFormat $(MKDIR) $(INSTALL_DIR)\include\FileFormat
	$(INSTALL) .\FileFormat\*.h $(INSTALL_DIR)\include\FileFormat
	IF NOT EXIST $(INSTALL_DIR)\include\FileFormat\AVS $(MKDIR) $(INSTALL_DIR)\include\FileFormat\AVS
	$(INSTALL) .\FileFormat\AVS\*.h $(INSTALL_DIR)\include\FileFormat\AVS
	IF NOT EXIST $(INSTALL_DIR)\include\FileFormat\CGNS $(MKDIR) $(INSTALL_DIR)\include\FileFormat\CGNS
	$(INSTALL) .\FileFormat\CGNS\*.h $(INSTALL_DIR)\include\FileFormat\CGNS
	IF NOT EXIST $(INSTALL_DIR)\include\FileFormat\KVSML $(MKDIR) $(INSTALL_DIR)\include\FileFormat\KVSML
	$(INSTALL) .\FileFormat\KVSML\*.h $(INSTALL_DIR)\include\FileFormat\KVSML
	IF NOT EXIST $(INSTALL_DIR)\include\FileFormat\PLOT3D $(MKDIR) $(INSTALL_DIR)\include\FileFormat\PLOT3D
	$(INSTALL) .\FileFormat\PLOT3D\*.h $(INSTALL_DIR)\include\FileFormat\PLOT3D
	IF NOT EXIST $(INSTALL_DIR)\include\FileFormat\STL $(MKDIR) $(INSTALL_DIR)\include\FileFormat\STL
	$(INSTALL) .\FileFormat\STL\*.h $(INSTALL_DIR)\include\FileFormat\STL
	IF NOT EXIST $(INSTALL_DIR)\include\FileFormat\VTK $(MKDIR) $(INSTALL_DIR)\include\FileFormat\VTK
	$(INSTALL) .\FileFormat\VTK\*.h $(INSTALL_DIR)\include\FileFormat\VTK
#
	IF NOT EXIST $(INSTALL_DIR)\include\Importer $(MKDIR) $(INSTALL_DIR)\include\Importer
	$(INSTALL) .\Importer\*.h $(INSTALL_DIR)\include\Importer
	IF NOT EXIST $(INSTALL_DIR)\include\PBVRFileInformation $(MKDIR) $(INSTALL_DIR)\include\PBVRFileInformation
	$(INSTALL) .\PBVRFileInformation\*.h $(INSTALL_DIR)\include\PBVRFileInformation
	IF NOT EXIST $(INSTALL_DIR)\include\TimeSeriesFiles $(MKDIR) $(INSTALL_DIR)\include\TimeSeriesFiles
	$(INSTALL) .\TimeSeriesFiles\*.h $(INSTALL_DIR)\include\TimeSeriesFiles
	IF NOT EXIST $(INSTALL_DIR)\include\TimeSeriesFiles\EnSight $(MKDIR) $(INSTALL_DIR)\include\TimeSeriesFiles\EnSight
	$(INSTALL) .\TimeSeriesFiles\EnSight\*.h $(INSTALL_DIR)\include\TimeSeriesFiles\EnSight
