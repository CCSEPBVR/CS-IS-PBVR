/*----------------------------------------------------------------------------
 *
 *  Copyright (c) Visualization Laboratory, Kyoto University.
 *  All rights reserved.
 *  See http://www.viz.media.kyoto-u.ac.jp/kvs/copyright/ for details.
 */
#include "Convert.h"

#include <stdexcept>
#include <vector>

#include "kvs/PolygonObject"
#include "kvs/Type"
#include "kvs/ValueArray"
#include "kvs/Vector3"
#include <vtkCellData.h>
#include <vtkFloatArray.h>
#include <vtkNew.h>
#include <vtkPointData.h>
#include <vtkPolyData.h>
#include <vtkPolyDataNormals.h>
#include <vtkSmartPointer.h>
#include <vtkTypedArray.h>

namespace
{

template <typename VtkPointSetPointerType>
int GetCountOfComponents( VtkPointSetPointerType data )
{
    auto point_data = data->GetPointData();
    int component_count = 0;

    for ( vtkIdType i = 0; i < point_data->GetNumberOfArrays(); ++i )
    {
        auto array = point_data->GetArray( i );
        component_count += array->GetNumberOfComponents();
    }

    return component_count;
}

template <typename T>
kvs::AnyValueArray GetValueArrayImpl( vtkPointData* point_data, vtkIdType node_count,
                                      int component_count )
{
    if ( component_count == 0 )
    {
        kvs::ValueArray<T> t_values;

        return kvs::AnyValueArray( t_values );
    }
    else
    {
        kvs::ValueArray<T> t_values( component_count * node_count );
        int component_head = 0;

        for ( vtkIdType i = 0; i < point_data->GetNumberOfArrays(); ++i )
        {
            auto array = point_data->GetArray( i );

            for ( vtkIdType p = 0; p < node_count; ++p )
            {
                auto t = array->GetTuple( p );

                for ( int c = 0; c < array->GetNumberOfComponents(); ++c )
                {
                    t_values[( component_head + c ) * node_count + p] = static_cast<T>( t[c] );
                }
            }

            component_head += array->GetNumberOfComponents();
        }

        return kvs::AnyValueArray( t_values );
    }
}

template <typename VtkPointSetPointerType>
kvs::VolumeObjectBase::Values GetValueArray( VtkPointSetPointerType data, int component_count )
{
    auto point_data = data->GetPointData();

    if ( component_count > 0 )
    {
        auto first_array = point_data->GetArray( 0 );

        switch ( first_array->GetDataType() )
        {
        case VTK_FLOAT:
            return ::GetValueArrayImpl<kvs::Real32>( data->GetPointData(),
                                                     data->GetNumberOfPoints(), component_count );
        case VTK_TYPE_INT8:
            return ::GetValueArrayImpl<kvs::Int8>( data->GetPointData(), data->GetNumberOfPoints(),
                                                   component_count );
        case VTK_TYPE_INT16:
            return ::GetValueArrayImpl<kvs::Int16>( data->GetPointData(), data->GetNumberOfPoints(),
                                                    component_count );
        case VTK_TYPE_INT32:
            return ::GetValueArrayImpl<kvs::Int32>( data->GetPointData(), data->GetNumberOfPoints(),
                                                    component_count );
        case VTK_TYPE_INT64:
            return ::GetValueArrayImpl<kvs::Int64>( data->GetPointData(), data->GetNumberOfPoints(),
                                                    component_count );
        case VTK_TYPE_UINT8:
            return ::GetValueArrayImpl<kvs::UInt8>( data->GetPointData(), data->GetNumberOfPoints(),
                                                    component_count );
        case VTK_TYPE_UINT16:
            return ::GetValueArrayImpl<kvs::UInt16>( data->GetPointData(),
                                                     data->GetNumberOfPoints(), component_count );
        case VTK_TYPE_UINT32:
            return ::GetValueArrayImpl<kvs::UInt32>( data->GetPointData(),
                                                     data->GetNumberOfPoints(), component_count );
        case VTK_TYPE_UINT64:
            return ::GetValueArrayImpl<kvs::UInt64>( data->GetPointData(),
                                                     data->GetNumberOfPoints(), component_count );
        case VTK_DOUBLE:
        default:
            return ::GetValueArrayImpl<kvs::Real64>( data->GetPointData(),
                                                     data->GetNumberOfPoints(), component_count );
        }
    }
    else
    {
        return ::GetValueArrayImpl<kvs::Real64>( data->GetPointData(), data->GetNumberOfPoints(),
                                                 0 );
    }
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

void cvt::detail::ConvertToPolygonObject( kvs::PolygonObject* polygon_object,
                                          vtkSmartPointer<vtkPolyData> data )
{
    std::vector<kvs::Real32> coords( data->GetNumberOfCells() * 3 * 3 );

    auto c_head = coords.data();
    for ( vtkIdType i = 0; i < data->GetNumberOfCells(); ++i )
    {
        auto cell = data->GetCell( i );

        for ( vtkIdType j = 0; j < cell->GetNumberOfPoints(); ++j )
        {
            auto p = data->GetPoint( cell->GetPointId( j ) );

            *c_head = static_cast<kvs::Real32>( p[0] );
            *( c_head + 1 ) = static_cast<kvs::Real32>( p[1] );
            *( c_head + 2 ) = static_cast<kvs::Real32>( p[2] );

            c_head += 3;
        }
    }

    std::vector<kvs::Real32> normals( data->GetNumberOfCells() * 3 );
    auto stl_normals = data->GetCellData()->GetNormals();

    if ( !stl_normals )
    {
        stl_normals = data->GetCellData()->GetArray( "cellNormals" );
    }

    if ( !stl_normals )
    {
        vtkNew<vtkPolyDataNormals> normals_filter;

        normals_filter->SetInputData( data );
        normals_filter->ComputeCellNormalsOn();
        normals_filter->ComputePointNormalsOff();
        normals_filter->SetOutputPointsPrecision(
            vtkAlgorithm::DesiredOutputPrecision::SINGLE_PRECISION );

        normals_filter->Update();

        data = normals_filter->GetOutput();
        stl_normals = data->GetCellData()->GetArray( "Normals" );

        if ( !stl_normals )
        {
            throw std::runtime_error( "failed to read normals" );
        }
    }

    auto n_head = normals.begin();
    for ( vtkIdType i = 0; i < data->GetNumberOfCells(); ++i )
    {
        auto n = stl_normals->GetTuple( i );

        *n_head = static_cast<kvs::Real32>( n[0] );
        *( n_head + 1 ) = static_cast<kvs::Real32>( n[1] );
        *( n_head + 2 ) = static_cast<kvs::Real32>( n[2] );

        n_head += 3;
    }

    auto v_normals = kvs::ValueArray<kvs::Real32>( normals );
    auto v_coords = kvs::ValueArray<kvs::Real32>( coords );

    polygon_object->setPolygonType( kvs::PolygonObject::Triangle );
    polygon_object->setNormalType( kvs::PolygonObject::PolygonNormal );
    polygon_object->setCoords( v_coords );
    polygon_object->setNormals( v_normals );
    polygon_object->updateMinMaxCoords();
}

void cvt::detail::ConvertFromPolygonObject( vtkSmartPointer<vtkPolyData>& data,
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

void cvt::detail::ConvertToRectilinearObject( kvs::StructuredVolumeObject* rectilinear_object,
                                              vtkSmartPointer<vtkRectilinearGrid> data )
{

    kvs::Vector3ui resolution;
    auto dimensions = data->GetDimensions();
    resolution[0] = static_cast<unsigned int>( dimensions[0] );
    resolution[1] = static_cast<unsigned int>( dimensions[1] );
    resolution[2] = static_cast<unsigned int>( dimensions[2] );

    kvs::StructuredVolumeObject::Coords xyz_coords(
        std::accumulate( dimensions, dimensions + 3, 0, std::plus<int>() ) );
    auto coords_itr = xyz_coords.begin();

    {
        auto x = data->GetXCoordinates();
        for ( int i = 0; i < x->GetNumberOfTuples(); ++i )
        {
            *( coords_itr++ ) = static_cast<float>( x->GetTuple( i )[0] );
        }
    }
    {
        auto y = data->GetYCoordinates();
        for ( int i = 0; i < y->GetNumberOfTuples(); ++i )
        {
            *( coords_itr++ ) = static_cast<float>( y->GetTuple( i )[0] );
        }
    }
    {
        auto z = data->GetZCoordinates();
        for ( int i = 0; i < z->GetNumberOfTuples(); ++i )
        {
            *( coords_itr++ ) = static_cast<float>( z->GetTuple( i )[0] );
        }
    }

    rectilinear_object->setGridType( kvs::StructuredVolumeObject::Rectilinear );
    rectilinear_object->setResolution( resolution );
    rectilinear_object->setCoords( xyz_coords );

    auto component_count = ::GetCountOfComponents( data );
    rectilinear_object->setVeclen( component_count );
    if ( component_count != 0 )
    {
        auto values = ::GetValueArray( data, component_count );
        rectilinear_object->setValues( values );
    }

    rectilinear_object->updateMinMaxValues();
}

void cvt::detail::ConvertFromRectilinearObject(
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