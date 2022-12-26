/*----------------------------------------------------------------------------
 *
 *  Copyright (c) Visualization Laboratory, Kyoto University.
 *  All rights reserved.
 *  See http://www.viz.media.kyoto-u.ac.jp/kvs/copyright/ for details.
 */
#ifndef CVT__VTK_STRUCTURED_POINTS_H_INCLUDE
#define CVT__VTK_STRUCTURED_POINTS_H_INCLUDE
#include "kvs/StructuredVolumeObject"
#include <vtkImageData.h>
#include <vtkStructuredPoints.h>
#include <vtkStructuredPointsReader.h>
#include <vtkStructuredPointsWriter.h>

#include "FileFormat/VtkFileFormat.h"

namespace cvt
{

/**
 * A VTK StructuredPoints (ImageData) file IO.
 */
using VtkStructuredPoints =
    typename cvt::VtkFileFormat<vtkImageData, vtkStructuredPointsReader, vtkStructuredPointsWriter>;
} // namespace cvt

#endif // CVT__VTK_STRUCTURED_POINTS_H_INCLUDE