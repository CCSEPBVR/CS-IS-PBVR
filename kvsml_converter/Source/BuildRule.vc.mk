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

OBJECTS = \
$(OUTDIR)/PBVRFileInformation/UnstructuredPfi.obj \
$(OUTDIR)/Exporter/UnstructuredVolumeObjectExporter.obj \
$(OUTDIR)/Exporter/StructuredVolumeObjectExporter.obj \
$(OUTDIR)/FileFormat/VtkCompositeDataSetFileFormat.obj \
$(OUTDIR)/Importer/VtkImport.obj


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
