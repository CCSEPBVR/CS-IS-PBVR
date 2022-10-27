/*----------------------------------------------------------------------------
 *
 *  Copyright (c) Visualization Laboratory, Kyoto University.
 *  All rights reserved.
 *  See http://www.viz.media.kyoto-u.ac.jp/kvs/copyright/ for details.
 */
#ifndef CVT__STL_H_INCLUDE
#define CVT__STL_H_INCLUDE
#include "FileFormat/VtkFileFormat.h"

#include <string>

#include "kvs/PolygonObject"
#include <vtkPolyData.h>
#include <vtkSTLReader.h>
#include <vtkSTLWriter.h>

namespace cvt
{

/**
 * A format tag for STL.
 */
using StlFormatTag = struct
{
};

template <>
struct VtkFileFormatTraits<cvt::StlFormatTag>
{
public:
    using VtkDataType = vtkPolyData;
    using KvsDataType = kvs::PolygonObject;
    using VtkReaderType = vtkSTLReader;
    using VtkWriterType = vtkSTLWriter;
};

/**
 * A STL file IO and converter.
 */
using Stl = typename cvt::VtkFileFormat<cvt::StlFormatTag>;

template <>
inline bool CheckExtension<cvt::Stl>( const std::string& filename )
{
    const char* const FILE_EXTENSIONS[] = { "stl", "stla", "stlb", "sla", "slb" };

    return cvt::detail::CheckExtensionImpl( filename, FILE_EXTENSIONS,
                                            sizeof( FILE_EXTENSIONS ) /
                                                sizeof( decltype( FILE_EXTENSIONS ) ) );
}
} // namespace cvt

#endif // CVT__STL_H_INCLUDE