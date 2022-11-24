/*----------------------------------------------------------------------------
 *
 *  Copyright (c) Visualization Laboratory, Kyoto University.
 *  All rights reserved.
 *  See http://www.viz.media.kyoto-u.ac.jp/kvs/copyright/ for details.
 */
#ifndef CVT__UNIFORM_VTK_XML_PSTRUCTURED_GRID_H_INCLUDE
#define CVT__UNIFORM_VTK_XML_PSTRUCTURED_GRID_H_INCLUDE
#include "kvs/StructuredVolumeObject"
#include <vtkStructuredGrid.h>
#include <vtkXMLPStructuredGridReader.h>
#include <vtkXMLPStructuredGridWriter.h>

#include "FileFormat/ParallelVtkFileFormat.h"
#include "FileFormat/UniformVTKXMLStructuredGrid/UniformVtkXmlStructuredGrid.h"

namespace cvt
{

/**
 * A VTK XML Parallel StructuredGrid file IO for an uniform grid.
 */
using UniformVtkXmlPStructuredGrid =
    typename cvt::ParallelVtkFileFormat<cvt::UniformVtkXmlStructuredGrid, vtkStructuredGrid,
                                        vtkXMLPStructuredGridReader, vtkXMLPStructuredGridWriter>;
} // namespace cvt

#endif // CVT__UNIFORM_VTK_XML_PSTRUCTURED_GRID_H_INCLUDE
