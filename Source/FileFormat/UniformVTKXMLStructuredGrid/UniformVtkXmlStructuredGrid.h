/*----------------------------------------------------------------------------
 *
 *  Copyright (c) Visualization Laboratory, Kyoto University.
 *  All rights reserved.
 *  See http://www.viz.media.kyoto-u.ac.jp/kvs/copyright/ for details.
 */
#ifndef CVT__UNIFORM_VTK_XML_STRUCTURED_GRID_H_INCLUDE
#define CVT__UNIFORM_VTK_XML_STRUCTURED_GRID_H_INCLUDE
#include "kvs/StructuredVolumeObject"
#include <vtkStructuredGrid.h>
#include <vtkXMLStructuredGridReader.h>
#include <vtkXMLStructuredGridWriter.h>

#include "FileFormat/VtkFileFormat.h"

namespace cvt
{

/**
 * A VTK XML StructuredGrid file IO for an uniform grid.
 */
using UniformVtkXmlStructuredGrid =
    typename cvt::VtkFileFormat<vtkStructuredGrid, vtkXMLStructuredGridReader,
                                vtkXMLStructuredGridWriter>;
} // namespace cvt

#endif // CVT__UNIFORM_VTK_XML_STRUCTURED_GRID_H_INCLUDE