/*----------------------------------------------------------------------------
 *
 *  Copyright (c) Visualization Laboratory, Kyoto University.
 *  All rights reserved.
 *  See http://www.viz.media.kyoto-u.ac.jp/kvs/copyright/ for details.
 */
#ifndef CVT__VTK_UNSTRUCTURED_GRID_H_INCLUDE
#define CVT__VTK_UNSTRUCTURED_GRID_H_INCLUDE
#include "kvs/UnstructuredVolumeObject"

#include <vtkUnstructuredGrid.h>
#include <vtkUnstructuredGridReader.h>
#include <vtkUnstructuredGridWriter.h>

#include "FileFormat/VtkUnstructuredFileFormat.h"

namespace cvt
{

/**
 * A VTK UnstructuredGrid file IO.
 */
using VtkUnstructuredGrid =
    typename cvt::VtkUnstructuredFileFormat<vtkUnstructuredGrid, vtkUnstructuredGridReader,
                                            vtkUnstructuredGridWriter>;
} // namespace cvt

#endif // CVT__VTK_UNSTRUCTURED_GRID_H_INCLUDE