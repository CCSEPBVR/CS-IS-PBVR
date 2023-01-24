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
#include "VtkExport.h"

#include <stdexcept>
#include <vector>

#include "kvs/PolygonObject"
#include "kvs/Type"
#include "kvs/ValueArray"
#include <vtkCellData.h>
#include <vtkFloatArray.h>
#include <vtkNew.h>
#include <vtkPointData.h>
#include <vtkPolyData.h>
#include <vtkPolyDataNormals.h>
#include <vtkSmartPointer.h>

namespace
{
template <typename KvsObjectPointerType>
vtkSmartPointer<vtkPoints> GetPoints( KvsObjectPointerType object, int point_count )
{
    vtkNew<vtkPoints> points;

    points->SetDataTypeToFloat();
    auto coords = object->coords().data();
    points->SetNumberOfPoints( static_cast<vtkIdType>( point_count ) );

    for ( vtkIdType i = 0; i < static_cast<vtkIdType>( point_count ); ++i )
    {
        points->SetPoint( i, *( coords + i * 3 ), *( coords + i * 3 + 1 ),
                          *( coords + i * 3 + 2 ) );
    }

    return points;
}

template <typename T>
void SetArraysToPointSetImpl( vtkPointData* point_data, const kvs::AnyValueArray& values,
                              vtkIdType node_count )
{
    auto value_array = values.asValueArray<T>();
    int component_count = values.size() / node_count;

    for ( int i = 0; i < component_count; ++i )
    {
        vtkNew<vtkAOSDataArrayTemplate<T>> array;
        array->SetNumberOfComponents( 1 );
        array->SetNumberOfTuples( node_count );

        for ( int p = 0; p < node_count; ++p )
        {
            array->SetValue( p, value_array[node_count * i + p] );
        }

        point_data->AddArray( array );
    }
}

void SetArraysToPointSet( vtkPointData* point_data, const kvs::AnyValueArray& values,
                          vtkIdType node_count )
{
    switch ( values.typeID() )
    {
    case kvs::Type::TypeID::TypeReal32:
        ::SetArraysToPointSetImpl<kvs::Real32>( point_data, values, node_count );
        break;
    case kvs::Type::TypeID::TypeInt8:
        ::SetArraysToPointSetImpl<kvs::Int8>( point_data, values, node_count );
        break;
    case kvs::Type::TypeID::TypeInt16:
        ::SetArraysToPointSetImpl<kvs::Int16>( point_data, values, node_count );
        break;
    case kvs::Type::TypeID::TypeInt32:
        ::SetArraysToPointSetImpl<kvs::Int32>( point_data, values, node_count );
        break;
    case kvs::Type::TypeID::TypeInt64:
        ::SetArraysToPointSetImpl<kvs::Int64>( point_data, values, node_count );
        break;
    case kvs::Type::TypeID::TypeUInt8:
        ::SetArraysToPointSetImpl<kvs::UInt8>( point_data, values, node_count );
        break;
    case kvs::Type::TypeID::TypeUInt16:
        ::SetArraysToPointSetImpl<kvs::UInt16>( point_data, values, node_count );
        break;
    case kvs::Type::TypeID::TypeUInt32:
        ::SetArraysToPointSetImpl<kvs::UInt32>( point_data, values, node_count );
        break;
    case kvs::Type::TypeID::TypeUInt64:
        ::SetArraysToPointSetImpl<kvs::UInt64>( point_data, values, node_count );
        break;
    case kvs::Type::TypeID::TypeReal64:
    default:
        ::SetArraysToPointSetImpl<kvs::Real64>( point_data, values, node_count );
    }
}
} // namespace

void cvt::detail::ExportPolygonObject( vtkSmartPointer<vtkPolyData>& data,
                                       const kvs::PolygonObject* polygon_object )
{
    vtkNew<vtkPolyData> p;
    auto points = ::GetPoints( polygon_object, polygon_object->numberOfVertices() );

    p->SetPoints( points );

    if ( polygon_object->numberOfConnections() == 0 )
    {
        vtkNew<vtkCellArray> cell_array;

        for ( vtkIdType i = 0; i < static_cast<vtkIdType>( polygon_object->numberOfVertices() / 3 );
              ++i )
        {
            vtkIdType ids[3] = { i * 3, i * 3 + 1, i * 3 + 2 };
            cell_array->InsertNextCell( 3, ids );
        }

        p->SetPolys( cell_array );
    }
    else
    {
        auto cell_count = polygon_object->connections().size() / 3;
        throw std::runtime_error( "not implemented" );
    }

    data = p;
}

void cvt::detail::ExportRectilinearGridObject(
    vtkSmartPointer<vtkRectilinearGrid>& data,
    const kvs::StructuredVolumeObject* rectilinear_object )
{
    if ( rectilinear_object->gridType() != kvs::StructuredVolumeObject::Rectilinear )
    {
        throw std::runtime_error( "A KVS structured volume object was not a rectilinear grid." );
        return;
    }

    vtkNew<vtkRectilinearGrid> grid;

    auto resolution = rectilinear_object->resolution();
    grid->SetDimensions( resolution[0], resolution[1], resolution[2] );

    auto coords = rectilinear_object->coords();

    vtkNew<vtkFloatArray> x_coords;
    x_coords->SetNumberOfComponents( 1 );
    x_coords->SetNumberOfTuples( resolution[0] );
    for ( unsigned int i = 0; i < resolution[0]; ++i )
    {
        x_coords->SetValue( i, coords[i] );
    }

    vtkNew<vtkFloatArray> y_coords;
    y_coords->SetNumberOfComponents( 1 );
    y_coords->SetNumberOfTuples( resolution[1] );
    for ( unsigned int i = 0; i < resolution[1]; ++i )
    {
        y_coords->SetValue( i, coords[i + resolution[0]] );
    }

    vtkNew<vtkFloatArray> z_coords;
    z_coords->SetNumberOfComponents( 1 );
    z_coords->SetNumberOfTuples( resolution[2] );
    for ( unsigned int i = 0; i < resolution[2]; ++i )
    {
        z_coords->SetValue( i, coords[i + resolution[0] + resolution[1]] );
    }

    grid->SetXCoordinates( x_coords );
    grid->SetYCoordinates( y_coords );
    grid->SetZCoordinates( z_coords );

    if ( rectilinear_object->veclen() > 0 )
    {
        auto point_data = grid->GetPointData();
        ::SetArraysToPointSet( point_data, rectilinear_object->values(),
                               grid->GetNumberOfPoints() );
    }

    data = grid;
}

void cvt::detail::ExportIrregularGridObject( vtkSmartPointer<vtkStructuredGrid>& data,
                                             const kvs::StructuredVolumeObject* irregular_object )
{
    vtkNew<vtkStructuredGrid> grid;

    auto resolution = irregular_object->resolution();
    grid->SetDimensions( resolution[0], resolution[1], resolution[2] );

    auto points = ::GetPoints( irregular_object, irregular_object->numberOfNodes() );
    grid->SetPoints( points );

    if ( irregular_object->veclen() > 0 )
    {
        auto point_data = grid->GetPointData();
        ::SetArraysToPointSet( point_data, irregular_object->values(), grid->GetNumberOfPoints() );
    }

    data = grid;
}

void cvt::detail::ExportUniformGridObject( vtkSmartPointer<vtkImageData>& data,
                                           const kvs::StructuredVolumeObject* uniform_object )
{
    vtkNew<vtkImageData> grid;

    auto resolution = uniform_object->resolution();
    grid->SetDimensions( resolution[0], resolution[1], resolution[2] );

    auto origin = uniform_object->minObjectCoord();
    auto max_coord = uniform_object->maxObjectCoord();
    grid->SetOrigin( static_cast<double>( origin[0] ), static_cast<double>( origin[1] ),
                     static_cast<double>( origin[2] ) );
    grid->SetSpacing(
        static_cast<double>( max_coord[0] - origin[0] ) / static_cast<double>( resolution[0] - 1 ),
        static_cast<double>( max_coord[1] - origin[1] ) / static_cast<double>( resolution[1] - 1 ),
        static_cast<double>( max_coord[2] - origin[2] ) /
            static_cast<double>( resolution[2] - 1 ) );

    if ( uniform_object->veclen() > 0 )
    {
        auto point_data = grid->GetPointData();
        ::SetArraysToPointSet( point_data, uniform_object->values(), grid->GetNumberOfPoints() );
    }

    data = grid;
}