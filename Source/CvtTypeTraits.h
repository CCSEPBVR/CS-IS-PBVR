/*----------------------------------------------------------------------------
 *
 *  Copyright (c) Visualization Laboratory, Kyoto University.
 *  All rights reserved.
 *  See http://www.viz.media.kyoto-u.ac.jp/kvs/copyright/ for details.
 */
#ifndef CVT__CVT_TYPE_TRAITS_H_INCLUDE
#define CVT__CVT_TYPE_TRAITS_H_INCLUDE

#include "kvs/PolygonObject"
#include "kvs/StructuredVolumeObject"
#include <vtkPolyData.h>
#include <vtkRectilinearGrid.h>
#include <vtkStructuredGrid.h>

namespace cvt
{

/**
 * A type traits for a convertible type.
 */
template <typename SourceType>
struct ConvertibleTypeTraits
{
    /**
     * A destination type.
     */
    using DestinationType = void;
};

template <>
struct ConvertibleTypeTraits<vtkPolyData>
{
    using DestinationType = kvs::PolygonObject;
};

template <>
struct ConvertibleTypeTraits<vtkStructuredGrid>
{
    using DestinationType = kvs::StructuredVolumeObject;
};

template <>
struct ConvertibleTypeTraits<vtkRectilinearGrid>
{
    using DestinationType = kvs::StructuredVolumeObject;
};
} // namespace cvt
#endif // CVT__CVT_TYPE_TRAITS_H_INCLUDE