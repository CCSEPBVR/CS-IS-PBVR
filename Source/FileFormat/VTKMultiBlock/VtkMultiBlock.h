/*----------------------------------------------------------------------------
 *
 *  Copyright (c) Visualization Laboratory, Kyoto University.
 *  All rights reserved.
 *  See http://www.viz.media.kyoto-u.ac.jp/kvs/copyright/ for details.
 */
#ifndef CVT__VTK_MULTI_BLOCK_H_INCLUDE
#define CVT__VTK_MULTI_BLOCK_H_INCLUDE
#include <vtkCompositeDataSet.h>
#include <vtkXMLMultiBlockDataReader.h>

#include "FileFormat/VtkCompositeDataSetFileFormat.h"

namespace cvt
{

using VtkMultiBlock =
    cvt::VtkCompositeDataSetFileFormat<vtkCompositeDataSet, vtkXMLMultiBlockDataReader>;
} // namespace cvt
#endif // CVT__VTK_MULTI_BLOCK_H_INCLUDE