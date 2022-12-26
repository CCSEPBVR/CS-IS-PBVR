/*----------------------------------------------------------------------------
 *
 *  Copyright (c) Visualization Laboratory, Kyoto University.
 *  All rights reserved.
 *  See http://www.viz.media.kyoto-u.ac.jp/kvs/copyright/ for details.
 */
#ifndef CVT__VTK_STRUCTURED_GRID_H_INCLUDE
#define CVT__VTK_STRUCTURED_GRID_H_INCLUDE
#include "kvs/StructuredVolumeObject"
#include <vtkStructuredGrid.h>
#include <vtkStructuredGridReader.h>
#include <vtkStructuredGridWriter.h>

#include "FileFormat/VtkFileFormat.h"

namespace cvt
{

/**
 * A VTK StructuredGrid file IO.
 */
using VtkStructuredGrid = typename cvt::VtkFileFormat<vtkStructuredGrid, vtkStructuredGridReader,
                                                      vtkStructuredGridWriter>;
} // namespace cvt

#endif // CVT__VTK_STRUCTURED_GRID_H_INCLUDE