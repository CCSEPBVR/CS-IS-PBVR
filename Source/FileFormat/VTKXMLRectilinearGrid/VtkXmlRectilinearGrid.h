/*----------------------------------------------------------------------------
 *
 *  Copyright (c) Visualization Laboratory, Kyoto University.
 *  All rights reserved.
 *  See http://www.viz.media.kyoto-u.ac.jp/kvs/copyright/ for details.
 */
#ifndef CVT__VTK_XML_RECTILINEAR_GRID_H_INCLUDE
#define CVT__VTK_XML_RECTILINEAR_GRID_H_INCLUDE
#include "FileFormat/VtkFileFormat.h"

#include "kvs/StructuredVolumeObject"
#include <vtkRectilinearGrid.h>
#include <vtkXMLRectilinearGridReader.h>
#include <vtkXMLRectilinearGridWriter.h>

namespace cvt
{

/**
 * A VTK RectilinearGrid file IO.
 */
using VtkXmlRectilinearGrid =
    typename cvt::VtkFileFormat<vtkRectilinearGrid, kvs::StructuredVolumeObject,
                                vtkXMLRectilinearGridReader, vtkXMLRectilinearGridWriter>;
} // namespace cvt

#endif // CVT__VTK_XML_RECTILINEAR_GRID_H_INCLUDE