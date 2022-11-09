/*----------------------------------------------------------------------------
 *
 *  Copyright (c) Visualization Laboratory, Kyoto University.
 *  All rights reserved.
 *  See http://www.viz.media.kyoto-u.ac.jp/kvs/copyright/ for details.
 */
#ifndef CVT__IRREGULAR_VTK_XML_STRUCTURED_GRID_H_INCLUDE
#define CVT__IRREGULAR_VTK_XML_STRUCTURED_GRID_H_INCLUDE
#include "kvs/StructuredVolumeObject"
#include <vtkStructuredGrid.h>
#include <vtkXMLStructuredGridReader.h>
#include <vtkXMLStructuredGridWriter.h>

#include "CvtTag.h"
#include "FileFormat/VtkFileFormat.h"

namespace cvt
{

/**
 * A VTK XML StructuredGrid file IO for an irregular grid.
 */
using IrregularVtkXmlStructuredGrid =
    typename cvt::VtkFileFormat<vtkStructuredGrid, kvs::StructuredVolumeObject,
                                vtkXMLStructuredGridReader, vtkXMLStructuredGridWriter,
                                cvt::IrregularGridTag>;
} // namespace cvt

#endif // CVT__IRREGULAR_VTK_XML_STRUCTURED_GRID_H_INCLUDE