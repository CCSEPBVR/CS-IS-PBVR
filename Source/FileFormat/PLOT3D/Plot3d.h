/*----------------------------------------------------------------------------
 *
 *  Copyright (c) Visualization Laboratory, Kyoto University.
 *  All rights reserved.
 *  See http://www.viz.media.kyoto-u.ac.jp/kvs/copyright/ for details.
 */
#ifndef CVT__PLOT3D_H_INCLUDE
#define CVT__PLOT3D_H_INCLUDE
#include <vtkMultiBlockDataSet.h>
#include <vtkMultiBlockPLOT3DReader.h>

#include "FileFormat/VtkCompositeDataSetFileFormat.h"

namespace cvt
{

/*
 * A PLOT3D file reader.
 */
using Plot3d = cvt::VtkCompositeDataSetFileFormat<vtkMultiBlockDataSet, vtkMultiBlockPLOT3DReader>;
} // namespace cvt
#endif // CVT__PLOT3D_H_INCLUDE