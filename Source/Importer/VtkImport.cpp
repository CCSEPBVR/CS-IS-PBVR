/*----------------------------------------------------------------------------
 *
 *  Copyright (c) Visualization Laboratory, Kyoto University.
 *  All rights reserved.
 *  See http://www.viz.media.kyoto-u.ac.jp/kvs/copyright/ for details.
 */
#include "VtkImport.h"

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
kvs::Vector3ui GetResolution( VtkPointSetPointerType data )
{
    kvs::Vector3ui resolution;

    auto dimensions = data->GetDimensions();
    resolution[0] = static_cast<unsigned int>( dimensions[0] );
    resolution[1] = static_cast<unsigned int>( dimensions[1] );
    resolution[2] = static_cast<unsigned int>( dimensions[2] );

    return resolution;
}

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
} // namespace

void cvt::detail::ImportPolygonObject( kvs::PolygonObject* polygon_object,
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

void cvt::detail::ImportRectilinearObject( kvs::StructuredVolumeObject* rectilinear_object,
                                           vtkSmartPointer<vtkRectilinearGrid> data )
{
    kvs::Vector3ui resolution = ::GetResolution( data );

    auto dimensions = data->GetDimensions();
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

void cvt::detail::ImportUniformStructuredVolumeObject( kvs::StructuredVolumeObject* uniform_object,
                                                       vtkSmartPointer<vtkStructuredGrid> data )
{
    auto bounding_box = data->GetBounds();
    kvs::Vector3f min_obj_coord( bounding_box[0], bounding_box[2], bounding_box[4] );
    kvs::Vector3f max_obj_coord( bounding_box[1], bounding_box[3], bounding_box[5] );
    uniform_object->setMinMaxObjectCoords( min_obj_coord, max_obj_coord );

    uniform_object->setGridType( kvs::StructuredVolumeObject::Uniform );
    uniform_object->setResolution( ::GetResolution( data ) );

    auto component_count = ::GetCountOfComponents( data );
    uniform_object->setVeclen( component_count );
    if ( component_count != 0 )
    {
        auto values = ::GetValueArray( data, component_count );
        uniform_object->setValues( values );
        uniform_object->updateMinMaxValues();
    }
}
