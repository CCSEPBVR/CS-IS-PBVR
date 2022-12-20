/*----------------------------------------------------------------------------
 *
 *  Copyright (c) Visualization Laboratory, Kyoto University.
 *  All rights reserved.
 *  See http://www.viz.media.kyoto-u.ac.jp/kvs/copyright/ for details.
 */
#ifndef CVT__AVS_UCD_H_INCLUDE
#define CVT__AVS_UCD_H_INCLUDE
#include "kvs/UnstructuredVolumeObject"

#include <vtkAVSucdReader.h>
#include <vtkUnstructuredGrid.h>
#include <vtkXMLUnstructuredGridWriter.h>

#include "FileFormat/VtkUnstructuredFileFormat.h"

namespace cvt
{

/**
 * An AVS ucd file format reader.
 *
 * This class does not support AVS ucd file writing.
 * This class writes data as a VTU file format.
 */
using AvsUcd = typename cvt::VtkUnstructuredFileFormat<vtkUnstructuredGrid, vtkAVSucdReader,
                                                       vtkXMLUnstructuredGridWriter>;
} // namespace cvt

#endif // CVT__AVS_UCD_H_INCLUDE