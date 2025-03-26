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

#*****************************************************************************
#  $Id: Makefile.$make 62 2009-03-19 06:58:44Z naohisa.sakamoto $
#*****************************************************************************

#=============================================================================
#  Include.
#=============================================================================
include ../Makefile.def


#=============================================================================
#  INCLUDE_PATH, LIBRARY_PATH, LINK_LIBRARY, INSTALL_DIR.
#=============================================================================
INCLUDE_PATH := -I.
ifdef KVS_INCLUDE_PATH
INCLUDE_PATH += -I$(KVS_INCLUDE_PATH)
endif
ifdef VTK_INCLUDE_PATH
INCLUDE_PATH += -I$(VTK_INCLUDE_PATH)
endif

LIBRARY_PATH := 
ifdef KVS_LIBRARY_PATH
LIBRARY_PATH += -L$(KVS_LIBRARY_PATH)
endif
ifdef VTK_LIBRARY_PATH 
LIBRARY_PATH += -L$(VTK_LIBRARY_PATH)
endif

LINK_LIBRARY := $(KVS_LINK_LIBRARY) $(VTK_LINK_LIBRARY)
INSTALL_DIR  := $(INSTALL_PREFIX)


#=============================================================================
#  Include build rule.
#=============================================================================
include BuildRule.mk


#=============================================================================
#  Project name.
#=============================================================================
TARGET_LIB := $(OUTDIR)/$(LIB_KVSML_CONVERTER)


#=============================================================================
#  Build rule.
#=============================================================================
$(TARGET_LIB): $(OBJECTS)
	$(AR) $@ $^
	$(RANLIB) $@

LIBRARY_PATH := -L$(OUTDIR)
ifdef KVS_LIBRARY_PATH
LIBRARY_PATH += -L$(KVS_LIBRARY_PATH)
endif
ifdef VTK_LIBRARY_PATH 
LIBRARY_PATH += -L$(VTK_LIBRARY_PATH)
endif
LINK_LIBRARY :=  -lkvsmlconverter $(KVS_LINK_LIBRARY) $(VTK_LINK_LIBRARY)

ifeq "$(findstring CYGWIN,$(shell uname -s))" "CYGWIN"
TARGET_EXE := $(OUTDIR)/$(KVSML_CONVERTER_BIN).exe
else
TARGET_EXE := $(OUTDIR)/$(KVSML_CONVERTER_BIN)
endif

$(TARGET_EXE): $(OUTDIR)/kvsml-converter.o $(TARGET_LIB)
	$(LD) $(LDFLAGS) $(LIBRARY_PATH) -o $@ $^ $(LINK_LIBRARY)

ifdef CVT_ENABLE_MPI
ifeq "$(findstring CYGWIN,$(shell uname -s))" "CYGWIN"
TARGET_EXE_MPI := $(OUTDIR)/$(KVSML_CONVERTER_MPI_BIN).exe
else
TARGET_EXE_MPI := $(OUTDIR)/$(KVSML_CONVERTER_MPI_BIN)
endif

$(TARGET_EXE_MPI): $(OUTDIR)/kvsml-converter-mpi.o $(TARGET_LIB) $(MPI_OBJECTS)
	$(LD) $(LDFLAGS) $(LIBRARY_PATH) -o $@ $^ $(LINK_LIBRARY)
endif

#=============================================================================
#  build.
#=============================================================================
build:: $(TARGET_LIB)
#build:: $(TARGET_EXE)
ifdef CVT_ENABLE_MPI
build:: $(TARGET_EXE_MPI)
endif


#=============================================================================
#  clean.
#=============================================================================
clean::
	$(RMDIR) $(OUTDIR)


#=============================================================================
#  install.
#=============================================================================
install::
	$(MKDIR) $(INSTALL_DIR)/lib
	$(INSTALL) $(TARGET_LIB) $(INSTALL_DIR)/lib
	$(RANLIB) $(INSTALL_DIR)/lib/$(LIB_KVSML_CONVERTER)
#install::
#	$(MKDIR) $(INSTALL_DIR)/bin
#	$(INSTALL_EXE) $(TARGET_EXE) $(INSTALL_DIR)/bin
ifdef CVT_ENABLE_MPI
install::
	$(MKDIR) $(INSTALL_DIR)/bin
	$(INSTALL_EXE) $(TARGET_EXE_MPI) $(INSTALL_DIR)/bin
endif

#=============================================================================
#  uninstall.
#=============================================================================
uninstall:
	$(RMDIR) $(INSTALL_DIR)


#=============================================================================
#  reinstall.
#=============================================================================
reinstall: uninstall install
