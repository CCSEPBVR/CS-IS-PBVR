/*----------------------------------------------------------------------------
 *
 *  Copyright (c) Visualization Laboratory, Kyoto University.
 *  All rights reserved.
 *  See http://www.viz.media.kyoto-u.ac.jp/kvs/copyright/ for details.
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

    vtkNew<vtkPoints> points;
    p->SetPoints( points );
    points->SetDataTypeToFloat();
    auto coords = polygon_object->coords().data();
    auto point_count = polygon_object->numberOfVertices();
    points->SetNumberOfPoints( static_cast<vtkIdType>( point_count ) );

    for ( vtkIdType i = 0; i < static_cast<vtkIdType>( point_count ); ++i )
    {
        points->SetPoint( i, *( coords + i * 3 ), *( coords + i * 3 + 1 ),
                          *( coords + i * 3 + 2 ) );
    }

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

void cvt::detail::ExportUniformGridObject( vtkSmartPointer<vtkStructuredGrid>& data,
                                           const kvs::StructuredVolumeObject* uniform_object )
{
    vtkNew<vtkStructuredGrid> grid;

    auto resolution = uniform_object->resolution();
    grid->SetDimensions( resolution[0], resolution[1], resolution[2] );

    auto min_coords = uniform_object->minObjectCoord();
    auto max_coords = uniform_object->maxObjectCoord();
    vtkNew<vtkPoints> points;

    points->SetDataTypeToFloat();
    points->SetNumberOfPoints( uniform_object->numberOfNodes() );

    vtkIdType i = 0;
    kvs::Real32 d[3] = {
        ( max_coords[0] - min_coords[0] ) / static_cast<kvs::Real32>( resolution[0] ),
        ( max_coords[1] - min_coords[1] ) / static_cast<kvs::Real32>( resolution[1] ),
        ( max_coords[2] - min_coords[2] ) / static_cast<kvs::Real32>( resolution[2] )
    };

    for ( unsigned int z = 0; z < resolution[2]; ++z )
    {
        kvs::Real32 z_coord = min_coords[2] + d[2] * z;
        for ( unsigned int y = 0; y < resolution[1]; ++y )
        {
            kvs::Real32 y_coord = min_coords[1] + d[1] * y;
            for ( unsigned int x = 0; x < resolution[0]; ++x )
            {
                kvs::Real32 x_coord = min_coords[0] + d[0] * x;

                points->SetPoint( i++, x_coord, y_coord, z_coord );
            }
        }
    }

    grid->SetPoints( points );

    if ( uniform_object->veclen() > 0 )
    {
        auto point_data = grid->GetPointData();
        ::SetArraysToPointSet( point_data, uniform_object->values(), grid->GetNumberOfPoints() );
    }

    data = grid;
}