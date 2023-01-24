/*
 * Copyright (c) 2022 Japan Atomic Energy Agency
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License,
 * or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <https://www.gnu.org/licenses/>.
 */
#ifndef CVT__CVT_TYPE_TRAITS_H_INCLUDE
#define CVT__CVT_TYPE_TRAITS_H_INCLUDE

#include "kvs/PolygonObject"
#include "kvs/StructuredVolumeObject"
#include "kvs/UnstructuredVolumeObject"
#include <vtkImageData.h>
#include <vtkPolyData.h>
#include <vtkRectilinearGrid.h>
#include <vtkStructuredGrid.h>
#include <vtkUnstructuredGrid.h>

namespace cvt
{

/**
 * A type traits for a convertible type.
 *
 * \tparam SourceType A source type.
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
struct ConvertibleTypeTraits<vtkImageData>
{
    using DestinationType = kvs::StructuredVolumeObject;
};

template <>
struct ConvertibleTypeTraits<vtkRectilinearGrid>
{
    using DestinationType = kvs::StructuredVolumeObject;
};

template <>
struct ConvertibleTypeTraits<vtkUnstructuredGrid>
{
    using DestinationType = kvs::UnstructuredVolumeObject;
};
} // namespace cvt
#endif // CVT__CVT_TYPE_TRAITS_H_INCLUDE