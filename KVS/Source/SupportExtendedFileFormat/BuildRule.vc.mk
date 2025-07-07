# Copyright (c) 2013 Naohisa Sakamoto
# All rights reserved.
#
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions are met:
#
# * Redistributions of source code must retain the above copyright notice, this
#   list of conditions and the following disclaimer.
#
# * Redistributions in binary form must reproduce the above copyright notice,
#   this list of conditions and the following disclaimer in the documentation
#   and/or other materials provided with the distribution.
#
# * Neither the name of KVS nor the names of its
#   contributors may be used to endorse or promote products derived from
#   this software without specific prior written permission.
#
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
# AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
# IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
# DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
# FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
# DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
# SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
# CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
# OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
# OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
#
# Copyright (c) 2022 Japan Atomic Energy Agency
#
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License,
# or (at your option) any later version.
#
# This program is distributed in the hope that it will be useful, but
# WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
# General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program. If not, see <https://www.gnu.org/licenses/>.

#  DON'T EDIT THIS FILE.

# OBJECTS = \
#$(OUTDIR)/Converter/ConverterTaskOutput.obj \
#$(OUTDIR)/Converter/ConverterTaskInput.obj \
#$(OUTDIR)/Converter/ConverterTask.obj \
#$(OUTDIR)/Converter/ConverterInputs.obj \
#$(OUTDIR)/Exporter/UnstructuredVolumeObjectExporter.obj \
#$(OUTDIR)/Exporter/StructuredVolumeObjectExporter.obj \
#$(OUTDIR)/FileFormat/VtkCompositeDataSetFileFormat.obj \
#$(OUTDIR)/Importer/VtkImport.obj \
#$(OUTDIR)/PBVRFileInformation/UnstructuredPfi.obj

OBJECTS = \
$(OUTDIR)/Exporter/UnstructuredVolumeObjectExporter.obj \
$(OUTDIR)/Exporter/StructuredVolumeObjectExporter.obj \
$(OUTDIR)/FileFormat/VtkCompositeDataSetFileFormat.obj \
$(OUTDIR)/Importer/VtkImport.obj

#$(OUTDIR)/kvsml-converter.obj:
#	IF NOT EXIST $(OUTDIR) $(MKDIR) $(OUTDIR)
#	$(CPP) /c $(CPPFLAGS) $(DEFINITIONS) $(INCLUDE_PATH) /Fo$(OUTDIR)/ ./kvsml-converter.cpp

#{.\PBVRFileInformation\}.cpp{$(OUTDIR)\PBVRFileInformation\}.obj::
#	IF NOT EXIST $(OUTDIR)\PBVRFileInformation $(MKDIR) $(OUTDIR)\PBVRFileInformation
#	$(CPP) /c $(CPPFLAGS) $(DEFINITIONS) $(INCLUDE_PATH) /Fo$(OUTDIR)\PBVRFileInformation\ @<<
#$<
#<<

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

#{.\Converter\}.cpp{$(OUTDIR)\Converter\}.obj::
#	IF NOT EXIST $(OUTDIR)\Converter $(MKDIR) $(OUTDIR)\Converter
#	$(CPP) /c $(CPPFLAGS) $(DEFINITIONS) $(INCLUDE_PATH) /Fo$(OUTDIR)\Converter\ @<<
#$<
#<<


#!IFDEF CVT_ENABLE_MPI
#MPI_OBJECTS = \
#$(OUTDIR)/kvsml-converter-mpi.obj \
#$(OUTDIR)/MPIRunner/MpiMainProcess.obj \
#$(OUTDIR)/MPIRunner/MpiSubProcess.obj


#{.\MPIRunner\}.cpp{$(OUTDIR)\MPIRunner\}.obj::
#	IF NOT EXIST $(OUTDIR)\MPIRunner $(MKDIR) $(OUTDIR)\MPIRunner
#	$(CPP) /c $(CPPFLAGS) $(DEFINITIONS) $(INCLUDE_PATH) /Fo$(OUTDIR)\MPIRunner\ @<<
#$<
#<<

#$(OUTDIR)/kvsml-converter-mpi.obj: ./kvsml-converter-mpi.cpp
#	IF NOT EXIST $(OUTDIR) $(MKDIR) $(OUTDIR)
#	$(CPP) /c $(CPPFLAGS) $(DEFINITIONS) $(INCLUDE_PATH) /Fo$(OUTDIR)/ ./kvsml-converter-mpi.cpp
#!ENDIF


install::
	IF NOT EXIST $(INSTALL_DIR)\include\SupportExtendedFileFormat $(MKDIR) $(INSTALL_DIR)\include\SupportExtendedFileFormat
	$(INSTALL) .\*.h $(INSTALL_DIR)\include\SupportExtendedFileFormat
	IF NOT EXIST $(INSTALL_DIR)\include\SupportExtendedFileFormat\Exporter $(MKDIR) $(INSTALL_DIR)\include\SupportExtendedFileFormat\Exporter
	$(INSTALL) .\Exporter\*.h $(INSTALL_DIR)\include\SupportExtendedFileFormat\Exporter
# FileFormat
	IF NOT EXIST $(INSTALL_DIR)\include\SupportExtendedFileFormat\FileFormat $(MKDIR) $(INSTALL_DIR)\include\SupportExtendedFileFormat\FileFormat
	$(INSTALL) .\FileFormat\*.h $(INSTALL_DIR)\include\SupportExtendedFileFormat\FileFormat
	IF NOT EXIST $(INSTALL_DIR)\include\SupportExtendedFileFormat\FileFormat\AVS $(MKDIR) $(INSTALL_DIR)\include\SupportExtendedFileFormat\FileFormat\AVS
	$(INSTALL) .\FileFormat\AVS\*.h $(INSTALL_DIR)\include\SupportExtendedFileFormat\FileFormat\AVS
	IF NOT EXIST $(INSTALL_DIR)\include\SupportExtendedFileFormat\FileFormat\CGNS $(MKDIR) $(INSTALL_DIR)\include\SupportExtendedFileFormat\FileFormat\CGNS
	$(INSTALL) .\FileFormat\CGNS\*.h $(INSTALL_DIR)\include\SupportExtendedFileFormat\FileFormat\CGNS
	IF NOT EXIST $(INSTALL_DIR)\include\SupportExtendedFileFormat\FileFormat\KVSML $(MKDIR) $(INSTALL_DIR)\include\SupportExtendedFileFormat\FileFormat\KVSML
	$(INSTALL) .\FileFormat\KVSML\*.h $(INSTALL_DIR)\include\SupportExtendedFileFormat\FileFormat\KVSML
	IF NOT EXIST $(INSTALL_DIR)\include\SupportExtendedFileFormat\FileFormat\PLOT3D $(MKDIR) $(INSTALL_DIR)\include\SupportExtendedFileFormat\FileFormat\PLOT3D
	$(INSTALL) .\FileFormat\PLOT3D\*.h $(INSTALL_DIR)\include\SupportExtendedFileFormat\FileFormat\PLOT3D
	IF NOT EXIST $(INSTALL_DIR)\include\SupportExtendedFileFormat\FileFormat\STL $(MKDIR) $(INSTALL_DIR)\include\SupportExtendedFileFormat\FileFormat\STL
	$(INSTALL) .\FileFormat\STL\*.h $(INSTALL_DIR)\include\SupportExtendedFileFormat\FileFormat\STL
	IF NOT EXIST $(INSTALL_DIR)\include\SupportExtendedFileFormat\FileFormat\VTK $(MKDIR) $(INSTALL_DIR)\include\SupportExtendedFileFormat\FileFormat\VTK
	$(INSTALL) .\FileFormat\VTK\*.h $(INSTALL_DIR)\include\SupportExtendedFileFormat\FileFormat\VTK
#
	IF NOT EXIST $(INSTALL_DIR)\include\SupportExtendedFileFormat\Importer $(MKDIR) $(INSTALL_DIR)\include\SupportExtendedFileFormat\Importer
	$(INSTALL) .\Importer\*.h $(INSTALL_DIR)\include\SupportExtendedFileFormat\Importer
#	IF NOT EXIST $(INSTALL_DIR)\include\SupportExtendedFileFormat\PBVRFileInformation $(MKDIR) $(INSTALL_DIR)\include\SupportExtendedFileFormat\PBVRFileInformation
#	$(INSTALL) .\PBVRFileInformation\*.h $(INSTALL_DIR)\include\SupportExtendedFileFormat\PBVRFileInformation
	IF NOT EXIST $(INSTALL_DIR)\include\SupportExtendedFileFormat\TimeSeriesFiles $(MKDIR) $(INSTALL_DIR)\include\SupportExtendedFileFormat\TimeSeriesFiles
	$(INSTALL) .\TimeSeriesFiles\*.h $(INSTALL_DIR)\include\SupportExtendedFileFormat\TimeSeriesFiles
	IF NOT EXIST $(INSTALL_DIR)\include\SupportExtendedFileFormat\TimeSeriesFiles\EnSight $(MKDIR) $(INSTALL_DIR)\include\SupportExtendedFileFormat\TimeSeriesFiles\EnSight
	$(INSTALL) .\TimeSeriesFiles\EnSight\*.h $(INSTALL_DIR)\include\SupportExtendedFileFormat\TimeSeriesFiles\EnSight

#!IFDEF CVT_ENABLE_MPI
#install::
#	IF NOT EXIST $(INSTALL_DIR)\include\Converter $(MKDIR) $(INSTALL_DIR)\include\Converter
#	$(INSTALL) .\Converter\*.h $(INSTALL_DIR)\include\Converter
#	IF NOT EXIST $(INSTALL_DIR)\include\MPIRunner $(MKDIR) $(INSTALL_DIR)\include\MPIRunner
#	$(INSTALL) .\MPIRunner\*.h $(INSTALL_DIR)\include\MPIRunner
#!ENDIF
