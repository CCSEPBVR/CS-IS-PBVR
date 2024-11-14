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
#ifndef CVT__STL_H_INCLUDE
#define CVT__STL_H_INCLUDE
#include "FileFormat/VtkFileFormat.h"

#include <string>

#include "kvs/PolygonObject"
#include <vtkPolyData.h>
#include <vtkSTLReader.h>
#include <vtkSTLWriter.h>

namespace cvt
{

/**
 * A STL file IO and converter.
 */
using Stl = typename cvt::VtkFileFormat<vtkPolyData, vtkSTLReader, vtkSTLWriter>;
} // namespace cvt

#endif // CVT__STL_H_INCLUDE