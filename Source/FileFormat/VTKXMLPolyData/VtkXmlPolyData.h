/*----------------------------------------------------------------------------
 *
 *  Copyright (c) Visualization Laboratory, Kyoto University.
 *  All rights reserved.
 *  See http://www.viz.media.kyoto-u.ac.jp/kvs/copyright/ for details.
 */
#ifndef CVT__VTK_XML_POLYDATA_H_INCLUDE
#define CVT__VTK_XML_POLYDATA_H_INCLUDE
#include "FileFormat/VtkFileFormat.h"

#include <string>

#include "kvs/PolygonObject"
#include <vtkPolyData.h>
#include <vtkXMLPolyDataReader.h>
#include <vtkXMLPolyDataWriter.h>

namespace cvt
{

/**
 * A VTK XML PolyData IO.
 */
using VtkXmlPolyData =
    cvt::VtkFileFormat<vtkPolyData, kvs::PolygonObject, vtkXMLPolyDataReader, vtkXMLPolyDataWriter>;
} // namespace cvt

#endif // CVT__VTK_XML_POLYDATA_H_INCLUDE