/*----------------------------------------------------------------------------
 *
 *  Copyright (c) Visualization Laboratory, Kyoto University.
 *  All rights reserved.
 *  See http://www.viz.media.kyoto-u.ac.jp/kvs/copyright/ for details.
 */
#ifndef CVT__VTK_XML_MULTI_BLOCK_H_INCLUDE
#define CVT__VTK_XML_MULTI_BLOCK_H_INCLUDE
#include <vtkCompositeDataSet.h>
#include <vtkXMLMultiBlockDataReader.h>

#include "FileFormat/VtkCompositeDataSetFileFormat.h"

namespace cvt
{

/*
 * A VTK MultiBlock IO.
 */
using VtkXmlMultiBlock =
    cvt::VtkCompositeDataSetFileFormat<vtkCompositeDataSet, vtkXMLMultiBlockDataReader>;
} // namespace cvt
#endif // CVT__VTK_XML_MULTI_BLOCK_H_INCLUDE