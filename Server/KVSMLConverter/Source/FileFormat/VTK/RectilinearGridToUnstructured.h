/*
 * Copyright (c) 2026 Japan Atomic Energy Agency
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 */
#ifndef CVT_RECTILINEAR_GRID_TO_UNSTRUCTURED_H_INCLUDE
#define CVT_RECTILINEAR_GRID_TO_UNSTRUCTURED_H_INCLUDE

#include <cmath>
#include <cstdint>
#include <limits>
#include <stdexcept>
#include <string>
#include <vector>

#include <vtkCellType.h>
#include <vtkDataArray.h>
#include <vtkDataSet.h>
#include <vtkFieldData.h>
#include <vtkNew.h>
#include <vtkPointData.h>
#include <vtkPoints.h>
#include <vtkRectilinearGrid.h>
#include <vtkSmartPointer.h>
#include <vtkUnstructuredGrid.h>

namespace cvt
{
namespace detail
{

inline std::size_t CheckedGridProduct( std::size_t lhs, std::size_t rhs,
                                       const char* quantity )
{
    if ( lhs != 0 && rhs > std::numeric_limits<std::size_t>::max() / lhs )
    {
        throw std::overflow_error( std::string( quantity ) + " exceeds size_t" );
    }
    return lhs * rhs;
}

enum class AxisDirection
{
    Ascending,
    Descending
};

inline AxisDirection ValidateRectilinearAxis(
    const std::vector<double>& coordinates, const char* axis_name )
{
    if ( coordinates.size() < 2 )
    {
        throw std::invalid_argument( std::string( axis_name ) +
                                     " axis requires at least two coordinates for a 3D grid" );
    }

    for ( std::size_t i = 0; i < coordinates.size(); ++i )
    {
        if ( !std::isfinite( coordinates[i] ) )
        {
            throw std::invalid_argument( std::string( axis_name ) +
                                         " axis contains a non-finite coordinate" );
        }
        if ( i > 0 && coordinates[i - 1] == coordinates[i] )
        {
            throw std::invalid_argument( std::string( axis_name ) +
                                         " axis contains duplicate coordinates" );
        }
    }

    const AxisDirection direction = coordinates[1] > coordinates[0]
                                        ? AxisDirection::Ascending
                                        : AxisDirection::Descending;
    for ( std::size_t i = 2; i < coordinates.size(); ++i )
    {
        const bool is_ascending = coordinates[i] > coordinates[i - 1];
        if ( is_ascending != ( direction == AxisDirection::Ascending ) )
        {
            throw std::invalid_argument( std::string( axis_name ) +
                                         " axis is not strictly monotonic" );
        }
    }

    return direction;
}

} // namespace detail

/**
 * Converts separable rectilinear coordinates and point-centered data to linear
 * VTK hexahedra. Coordinate units and values are preserved without conversion.
 * Each axis must be finite and strictly monotonic. Descending axes keep their
 * point order; only each cell's connectivity is oriented from the physically
 * lower corner to the higher corner.
 *
 * The point order is i + nx * (j + ny * k). Point and field arrays are copied
 * shallowly from source_data; their lifetime is retained by VTK reference
 * counting. The caller may therefore use an image, rectilinear, or other VTK
 * data set as the source of point-centered arrays.
 */
inline vtkSmartPointer<vtkUnstructuredGrid> RectilinearGridToLinearHexahedra(
    const std::vector<double>& x_coordinates,
    const std::vector<double>& y_coordinates,
    const std::vector<double>& z_coordinates, vtkDataSet* source_data )
{
    if ( !source_data )
    {
        throw std::invalid_argument( "rectilinear conversion requires a source data set" );
    }

    const auto x_direction = detail::ValidateRectilinearAxis( x_coordinates, "x" );
    const auto y_direction = detail::ValidateRectilinearAxis( y_coordinates, "y" );
    const auto z_direction = detail::ValidateRectilinearAxis( z_coordinates, "z" );

    const std::size_t x0 = x_direction == detail::AxisDirection::Ascending ? 0 : 1;
    const std::size_t x1 = x_direction == detail::AxisDirection::Ascending ? 1 : 0;
    const std::size_t y0 = y_direction == detail::AxisDirection::Ascending ? 0 : 1;
    const std::size_t y1 = y_direction == detail::AxisDirection::Ascending ? 1 : 0;
    const std::size_t z0 = z_direction == detail::AxisDirection::Ascending ? 0 : 1;
    const std::size_t z1 = z_direction == detail::AxisDirection::Ascending ? 1 : 0;

    const std::size_t nx = x_coordinates.size();
    const std::size_t ny = y_coordinates.size();
    const std::size_t nz = z_coordinates.size();
    const std::size_t node_count = detail::CheckedGridProduct(
        detail::CheckedGridProduct( nx, ny, "rectilinear node count" ), nz,
        "rectilinear node count" );
    const std::size_t cell_count = detail::CheckedGridProduct(
        detail::CheckedGridProduct( nx - 1, ny - 1, "rectilinear cell count" ), nz - 1,
        "rectilinear cell count" );
    (void)detail::CheckedGridProduct( cell_count, 8, "rectilinear connection count" );

    if ( node_count > static_cast<std::size_t>( std::numeric_limits<vtkIdType>::max() ) ||
         cell_count > static_cast<std::size_t>( std::numeric_limits<vtkIdType>::max() ) )
    {
        throw std::overflow_error( "rectilinear grid exceeds the VTK identifier range" );
    }
    if ( node_count - 1 >
         static_cast<std::size_t>( std::numeric_limits<std::uint32_t>::max() ) )
    {
        throw std::overflow_error( "rectilinear connections exceed the KVS UInt32 range" );
    }

    const auto vtk_node_count = static_cast<vtkIdType>( node_count );
    if ( source_data->GetNumberOfPoints() != vtk_node_count )
    {
        throw std::invalid_argument(
            "point-data tuple count does not match nx * ny * nz" );
    }
    vtkPointData* point_data = source_data->GetPointData();
    if ( !point_data || point_data->GetNumberOfArrays() == 0 )
    {
        throw std::invalid_argument( "rectilinear conversion requires point-data arrays" );
    }
    for ( int i = 0; i < point_data->GetNumberOfArrays(); ++i )
    {
        vtkDataArray* array = point_data->GetArray( i );
        if ( !array || array->GetNumberOfTuples() != vtk_node_count )
        {
            throw std::invalid_argument(
                "a point-data array does not contain nx * ny * nz tuples" );
        }
    }

    vtkNew<vtkPoints> points;
    points->SetDataTypeToDouble();
    points->SetNumberOfPoints( vtk_node_count );
    for ( std::size_t k = 0; k < nz; ++k )
    {
        for ( std::size_t j = 0; j < ny; ++j )
        {
            for ( std::size_t i = 0; i < nx; ++i )
            {
                const std::size_t node = i + nx * ( j + ny * k );
                points->SetPoint( static_cast<vtkIdType>( node ), x_coordinates[i],
                                  y_coordinates[j], z_coordinates[k] );
            }
        }
    }

    vtkSmartPointer<vtkUnstructuredGrid> output =
        vtkSmartPointer<vtkUnstructuredGrid>::New();
    output->SetPoints( points );
    output->Allocate( static_cast<vtkIdType>( cell_count ) );
    for ( std::size_t k = 0; k + 1 < nz; ++k )
    {
        for ( std::size_t j = 0; j + 1 < ny; ++j )
        {
            for ( std::size_t i = 0; i + 1 < nx; ++i )
            {
                const auto node = [=]( std::size_t di, std::size_t dj,
                                       std::size_t dk ) -> vtkIdType {
                    return static_cast<vtkIdType>(
                        ( i + di ) + nx * ( ( j + dj ) + ny * ( k + dk ) ) );
                };
                const vtkIdType connection[8] = {
                    node( x0, y0, z0 ), node( x1, y0, z0 ),
                    node( x1, y1, z0 ), node( x0, y1, z0 ),
                    node( x0, y0, z1 ), node( x1, y0, z1 ),
                    node( x1, y1, z1 ), node( x0, y1, z1 )
                };
                output->InsertNextCell( VTK_HEXAHEDRON, 8, connection );
            }
        }
    }

    output->GetPointData()->ShallowCopy( point_data );
    if ( source_data->GetFieldData() )
    {
        output->GetFieldData()->ShallowCopy( source_data->GetFieldData() );
    }
    return output;
}

/**
 * Converts the physical coordinates emitted by a VTK rectilinear-grid reader
 * to linear hexahedra. This overload deliberately reads the coordinate arrays
 * from the reader output instead of reconstructing them from NetCDF metadata,
 * so that downstream geometry is identical to the geometry seen by ParaView.
 */
inline vtkSmartPointer<vtkUnstructuredGrid> RectilinearGridToLinearHexahedra(
    vtkRectilinearGrid* source_data )
{
    if ( !source_data )
    {
        throw std::invalid_argument( "rectilinear conversion requires a VTK rectilinear grid" );
    }

    int dimensions[3] = {};
    source_data->GetDimensions( dimensions );
    vtkDataArray* vtk_coordinates[3] = {
        source_data->GetXCoordinates(), source_data->GetYCoordinates(),
        source_data->GetZCoordinates()
    };
    const char* axis_names[3] = { "x", "y", "z" };
    std::vector<std::vector<double>> coordinates( 3 );
    for ( int axis = 0; axis < 3; ++axis )
    {
        if ( dimensions[axis] < 0 || !vtk_coordinates[axis] ||
             vtk_coordinates[axis]->GetNumberOfTuples() != dimensions[axis] )
        {
            throw std::invalid_argument( std::string( axis_names[axis] ) +
                                         " coordinate count does not match the VTK dimensions" );
        }
        coordinates[axis].resize( static_cast<std::size_t>( dimensions[axis] ) );
        for ( int i = 0; i < dimensions[axis]; ++i )
        {
            coordinates[axis][static_cast<std::size_t>( i )] =
                vtk_coordinates[axis]->GetComponent( i, 0 );
        }
    }

    return RectilinearGridToLinearHexahedra(
        coordinates[0], coordinates[1], coordinates[2], source_data );
}

} // namespace cvt

#endif // CVT_RECTILINEAR_GRID_TO_UNSTRUCTURED_H_INCLUDE
