/*----------------------------------------------------------------------------
 *
 *  Copyright (c) Visualization Laboratory, Kyoto University.
 *  All rights reserved.
 *  See http://www.viz.media.kyoto-u.ac.jp/kvs/copyright/ for details.
 */
#ifndef CVT__VTK_XML_IMAGE_DATA_H_INCLUDE
#define CVT__VTK_XML_IMAGE_DATA_H_INCLUDE
#include "kvs/StructuredVolumeObject"
#include <vtkImageData.h>
#include <vtkXMLImageDataReader.h>
#include <vtkXMLImageDataWriter.h>

#include "FileFormat/VtkFileFormat.h"

namespace cvt
{

/**
 * A VTK XML ImageData file IO.
 */
using VtkXmlImageData =
    typename cvt::VtkFileFormat<vtkImageData, vtkXMLImageDataReader, vtkXMLImageDataWriter>;
} // namespace cvt

#endif // CVT__VTK_XML_IMAGE_DATA_H_INCLUDE