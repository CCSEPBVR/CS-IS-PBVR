/*----------------------------------------------------------------------------
 *
 *  Copyright (c) Visualization Laboratory, Kyoto University.
 *  All rights reserved.
 *  See http://www.viz.media.kyoto-u.ac.jp/kvs/copyright/ for details.
 */
#ifndef CVT__VTK_POLYDATA_H_INCLUDE
#define CVT__VTK_POLYDATA_H_INCLUDE
#include "FileFormat/VtkFileFormat.h"

#include "kvs/PolygonObject"
#include <vtkPolyData.h>
#include <vtkPolyDataReader.h>
#include <vtkPolyDataWriter.h>

namespace cvt
{

/**
 * A VTK PolyData IO.
 */
using VtkPolyData = cvt::VtkFileFormat<vtkPolyData, vtkPolyDataReader, vtkPolyDataWriter>;
} // namespace cvt

#endif // CVT__VTK_POLYDATA_H_INCLUDE