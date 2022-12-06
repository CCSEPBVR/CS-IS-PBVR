/*----------------------------------------------------------------------------
 *
 *  Copyright (c) Visualization Laboratory, Kyoto University.
 *  All rights reserved.
 *  See http://www.viz.media.kyoto-u.ac.jp/kvs/copyright/ for details.
 */
#ifndef CVT__VTK_XML_UNSTRUCTURED_GRID_H_INCLUDE
#define CVT__VTK_XML_UNSTRUCTURED_GRID_H_INCLUDE
#include "kvs/UnstructuredVolumeObject"

#include <vtkUnstructuredGrid.h>
#include <vtkXMLUnstructuredGridReader.h>
#include <vtkXMLUnstructuredGridWriter.h>

#include "FileFormat/VtkUnstructuredFileFormat.h"

namespace cvt
{

/**
 * A VTK XML UnstructuredGrid file IO.
 */
using VtkXmlUnstructuredGrid =
    typename cvt::VtkUnstructuredFileFormat<vtkUnstructuredGrid, vtkXMLUnstructuredGridReader,
                                            vtkXMLUnstructuredGridWriter>;
} // namespace cvt

#endif // CVT__VTK_XML_UNSTRUCTURED_GRID_H_INCLUDE