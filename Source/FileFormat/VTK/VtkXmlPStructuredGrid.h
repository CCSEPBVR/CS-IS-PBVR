/*----------------------------------------------------------------------------
 *
 *  Copyright (c) Visualization Laboratory, Kyoto University.
 *  All rights reserved.
 *  See http://www.viz.media.kyoto-u.ac.jp/kvs/copyright/ for details.
 */
#ifndef CVT__VTK_XML_PSTRUCTURED_GRID_H_INCLUDE
#define CVT__VTK_XML_PSTRUCTURED_GRID_H_INCLUDE
#include "kvs/StructuredVolumeObject"
#include <vtkStructuredGrid.h>
#include <vtkXMLPStructuredGridReader.h>
#include <vtkXMLPStructuredGridWriter.h>

#include "FileFormat/ParallelVtkFileFormat.h"
#include "FileFormat/VTK/VtkXmlStructuredGrid.h"

namespace cvt
{

/**
 * A VTK XML Parallel StructuredGrid file IO.
 */
using VtkXmlPStructuredGrid =
    cvt::ParallelVtkFileFormat<cvt::VtkXmlStructuredGrid, vtkXMLPStructuredGridReader,
                               vtkXMLPStructuredGridWriter>;
} // namespace cvt

#endif // CVT__VTK_XML_PSTRUCTURED_GRID_H_INCLUDE
