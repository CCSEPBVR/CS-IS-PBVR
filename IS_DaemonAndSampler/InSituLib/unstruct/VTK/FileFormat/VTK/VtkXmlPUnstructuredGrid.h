/*
 * Copyright (c) 2022 Japan Atomic Energy Agency
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License,
 * or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <https://www.gnu.org/licenses/>.
 */
#ifndef CVT__VTK_XML_PUNSTRUCTURED_GRID_H_INCLUDE
#define CVT__VTK_XML_PUNSTRUCTURED_GRID_H_INCLUDE
#include "kvs/UnstructuredVolumeObject"

#include <vtkUnstructuredGrid.h>
#include <vtkXMLPUnstructuredGridReader.h>
#include <vtkXMLPUnstructuredGridWriter.h>

#include "FileFormat/ParallelVtkFileFormat.h"
#include "FileFormat/VTK/VtkXmlUnstructuredGrid.h"

namespace cvt
{

/**
 * A VTK XML Parallel UnstructuredGrid file IO.
 */
using VtkXmlPUnstructuredGrid =
    cvt::ParallelVtkFileFormat<cvt::VtkXmlUnstructuredGrid, vtkXMLPUnstructuredGridReader,
                               vtkXMLPUnstructuredGridWriter>;
} // namespace cvt

#endif // CVT__VTK_XML_PUNSTRUCTURED_GRID_H_INCLUDE
