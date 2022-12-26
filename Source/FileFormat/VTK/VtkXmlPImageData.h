/*----------------------------------------------------------------------------
 *
 *  Copyright (c) Visualization Laboratory, Kyoto University.
 *  All rights reserved.
 *  See http://www.viz.media.kyoto-u.ac.jp/kvs/copyright/ for details.
 */
#ifndef CVT__VTK_XML_P_IMAGE_DATA_H_INCLUDE
#define CVT__VTK_XML_P_IMAGE_DATA_H_INCLUDE
#include "kvs/StructuredVolumeObject"
#include <vtkStructuredGrid.h>
#include <vtkXMLPImageDataReader.h>
#include <vtkXMLPImageDataWriter.h>

#include "FileFormat/ParallelVtkFileFormat.h"
#include "FileFormat/VTK/VtkXmlImageData.h"

namespace cvt
{

/**
 * A VTK XML Parallel ImageData file IO.
 */
using VtkXmlPImageData = cvt::ParallelVtkFileFormat<cvt::VtkXmlImageData, vtkXMLPImageDataReader,
                                                    vtkXMLPImageDataWriter>;
} // namespace cvt

#endif // CVT__VTK_XML_P_IMAGE_DATA_H_INCLUDE