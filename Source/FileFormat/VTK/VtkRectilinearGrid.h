/*----------------------------------------------------------------------------
 *
 *  Copyright (c) Visualization Laboratory, Kyoto University.
 *  All rights reserved.
 *  See http://www.viz.media.kyoto-u.ac.jp/kvs/copyright/ for details.
 */
#ifndef CVT__VTK_RECTILINEAR_GRID_H_INCLUDE
#define CVT__VTK_RECTILINEAR_GRID_H_INCLUDE
#include "FileFormat/VtkFileFormat.h"

#include "kvs/StructuredVolumeObject"
#include <vtkRectilinearGrid.h>
#include <vtkRectilinearGridReader.h>
#include <vtkRectilinearGridWriter.h>

namespace cvt
{

/**
 * A VTK RectilinearGrid file IO.
 */
using VtkRectilinearGrid = typename cvt::VtkFileFormat<vtkRectilinearGrid, vtkRectilinearGridReader,
                                                       vtkRectilinearGridWriter>;
} // namespace cvt

#endif // CVT__VTK_RECTILINEAR_GRID_H_INCLUDE