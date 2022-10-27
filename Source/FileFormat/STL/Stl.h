/*----------------------------------------------------------------------------
 *
 *  Copyright (c) Visualization Laboratory, Kyoto University.
 *  All rights reserved.
 *  See http://www.viz.media.kyoto-u.ac.jp/kvs/copyright/ for details.
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
using Stl =
    typename cvt::VtkFileFormat<vtkPolyData, kvs::PolygonObject, vtkSTLReader, vtkSTLWriter>;
} // namespace cvt

#endif // CVT__STL_H_INCLUDE