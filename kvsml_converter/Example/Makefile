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
#  $Id: Makefile 1500 2013-04-04 07:56:40Z naohisa.sakamoto@gmail.com $
#*****************************************************************************

#=============================================================================
#  Include.
#=============================================================================
include ../Makefile.def


#=============================================================================
#  INCLUDE_PATH, LIBRARY_PATH, LINK_LIBRARY, INSTALL_DIR.
#=============================================================================
INCLUDE_PATH := -I../Source
ifdef KVS_INCLUDE_PATH
INCLUDE_PATH += -I$(KVS_INCLUDE_PATH)
endif
ifdef VTK_INCLUDE_PATH
INCLUDE_PATH += -I$(VTK_INCLUDE_PATH)
endif

LIBRARY_PATH := -L../Source/$(OUTDIR)
ifdef KVS_LIBRARY_PATH
LIBRARY_PATH += -L$(KVS_LIBRARY_PATH)
endif
ifdef VTK_LIBRARY_PATH 
LIBRARY_PATH += -L$(VTK_LIBRARY_PATH)
endif
LINK_LIBRARY := -lstdc++fs -lkvsmlconverter $(KVS_LINK_LIBRARY) $(VTK_LINK_LIBRARY)
INSTALL_DIR  := $(INSTALL_PREFIX)


#=============================================================================
#  Project name.
#=============================================================================
PROJECT_NAME := kvsml-converter

ifeq "$(findstring CYGWIN,$(shell uname -s))" "CYGWIN"
TARGET_EXE := $(OUTDIR)/$(PROJECT_NAME).exe
else
TARGET_EXE := $(OUTDIR)/$(PROJECT_NAME)
endif


#=============================================================================
#  Object.
#=============================================================================
OBJECTS := \
$(OUTDIR)/AvsExample.o \
$(OUTDIR)/CgnsExample.o \
$(OUTDIR)/EnSightExample.o \
$(OUTDIR)/Plot3dExample.o \
$(OUTDIR)/PvtsExample.o \
$(OUTDIR)/PvtuExample.o \
$(OUTDIR)/StlExample.o \
$(OUTDIR)/VtiExample.o \
$(OUTDIR)/VtmExample.o \
$(OUTDIR)/VtpExample.o \
$(OUTDIR)/VtrExample.o \
$(OUTDIR)/VtsExample.o \
$(OUTDIR)/VtuExample.o \
$(OUTDIR)/example.o


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
