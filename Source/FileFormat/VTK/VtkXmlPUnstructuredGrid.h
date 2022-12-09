/*----------------------------------------------------------------------------
 *
 *  Copyright (c) Visualization Laboratory, Kyoto University.
 *  All rights reserved.
 *  See http://www.viz.media.kyoto-u.ac.jp/kvs/copyright/ for details.
 */
#ifndef CVT__VTK_XML_PUNSTRUCTURED_GRID_H_INCLUDE
#define CVT__VTK_XML_PUNSTRUCTURED_GRID_H_INCLUDE
#include "kvs/UnstructuredVolumeObject"

#include <vtkUnstructuredGrid.h>
#include <vtkXMLPUnstructuredGridReader.h>
#include <vtkXMLPUnstructuredGridWriter.h>

#include "FileFormat/ParallelVtkFileFormat.h"
#include "FileFormat/VTK/VtkXmlUnstructuredGrid.h"

namespace cvt
{

/**
 * A VTK XML Parallel UnstructuredGrid file IO.
 */
using VtkXmlPUnstructuredGrid =
    cvt::ParallelVtkFileFormat<cvt::VtkXmlUnstructuredGrid, vtkXMLPUnstructuredGridReader,
                               vtkXMLPUnstructuredGridWriter>;
} // namespace cvt

#endif // CVT__VTK_XML_PUNSTRUCTURED_GRID_H_INCLUDE
