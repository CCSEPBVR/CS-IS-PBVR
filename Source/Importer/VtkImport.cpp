/*----------------------------------------------------------------------------
 *
 *  Copyright (c) Visualization Laboratory, Kyoto University.
 *  All rights reserved.
 *  See http://www.viz.media.kyoto-u.ac.jp/kvs/copyright/ for details.
 */
#include "VtkImport.h"

#include <deque>
#include <stdexcept>
#include <vector>

#include "kvs/LineObject"
#include "kvs/PointObject"
#include "kvs/PolygonObject"
#include "kvs/StructuredVolumeObject"
#include "kvs/Type"
#include "kvs/UnstructuredVolumeObject"
#include "kvs/ValueArray"
#include "kvs/Vector3"
#include <vtkCellArray.h>
#include <vtkCellData.h>
#include <vtkCellDataToPointData.h>
#include <vtkFloatArray.h>
#include <vtkNew.h>
#include <vtkPointData.h>
#include <vtkPolyData.h>
#include <vtkPolyDataNormals.h>
#include <vtkSmartPointer.h>
#include <vtkStructuredGrid.h>
#include <vtkTypedArray.h>
#include <vtkUnstructuredGrid.h>

namespace
{
template <typename VtkPointSetPointerType>
kvs::ValueArray<kvs::Real32> GetCoordinates( VtkPointSetPointerType data )
{
    std::vector<kvs::Real32> coords( data->GetNumberOfPoints() * 3 );

    auto c_head = coords.data();
    for ( vtkIdType i = 0; i < data->GetNumberOfPoints(); ++i )
    {
        auto p = data->GetPoint( i );

        *c_head = static_cast<kvs::Real32>( p[0] );
        *( c_head + 1 ) = static_cast<kvs::Real32>( p[1] );
        *( c_head + 2 ) = static_cast<kvs::Real32>( p[2] );

        c_head += 3;
    }

    return kvs::ValueArray<kvs::Real32>( coords );
}

template <typename VtkPointSetPointerType>
kvs::Vector3ui GetResolution( VtkPointSetPointerType data )
{
    kvs::Vector3ui resolution;

    int dimensions[3];
    data->GetDimensions( dimensions );
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

kvs::UnstructuredVolumeObject::CellType GetKvsCellType( int type )
{
    switch ( type )
    {
    case VTK_TETRA:
        return kvs::UnstructuredVolumeObject::Tetrahedra;
    case VTK_QUADRATIC_TETRA:
        return kvs::UnstructuredVolumeObject::QuadraticTetrahedra;
    case VTK_HEXAHEDRON:
        return kvs::UnstructuredVolumeObject::Hexahedra;
    case VTK_QUADRATIC_HEXAHEDRON:
        return kvs::UnstructuredVolumeObject::QuadraticHexahedra;
    case VTK_VERTEX:
        return kvs::UnstructuredVolumeObject::Point;
    case VTK_PYRAMID:
        return kvs::UnstructuredVolumeObject::Pyramid;
    case VTK_WEDGE:
        return kvs::UnstructuredVolumeObject::Prism;
    default:
        return kvs::UnstructuredVolumeObject::UnknownCellType;
    }
}

template <typename K, typename V, typename O>
void Reorder( K& kvs, V& vtk, O& order, int n )
{
    for ( int i = 0; i < n; ++i )
    {
        *kvs = vtk->GetId( order[i] );
        ++kvs;
    }
}

template <typename DestinationIterator>
void ReorderElementNodeIndices( DestinationIterator& kvs_connection, vtkIdList* vtk_cell,
                                int vtk_cell_type )
{
    switch ( vtk_cell_type )
    {
    case VTK_TRIANGLE: {
        constexpr int order[] = { 0, 1, 2 };
        ::Reorder( kvs_connection, vtk_cell, order, sizeof( order ) / sizeof( int ) );
        break;
    }
    case VTK_PIXEL: {
        constexpr int order[] = { 0, 1, 3, 2 };
        ::Reorder( kvs_connection, vtk_cell, order, sizeof( order ) / sizeof( int ) );
        break;
    }
    case VTK_QUAD: {
        constexpr int order[] = { 0, 1, 2, 3 };
        ::Reorder( kvs_connection, vtk_cell, order, sizeof( order ) / sizeof( int ) );
        break;
    }
    case VTK_TETRA: {
        constexpr int order[] = { 0, 3, 2, 1 };
        ::Reorder( kvs_connection, vtk_cell, order, sizeof( order ) / sizeof( int ) );
        break;
    }
    case VTK_QUADRATIC_TETRA: {
        constexpr int order[] = { 0, 3, 2, 1, 7, 6, 4, 9, 5, 8 };
        ::Reorder( kvs_connection, vtk_cell, order, sizeof( order ) / sizeof( int ) );
        break;
    }
    case VTK_HEXAHEDRON: {
        constexpr int order[] = { 4, 5, 6, 7, 0, 1, 2, 3 };
        ::Reorder( kvs_connection, vtk_cell, order, sizeof( order ) / sizeof( int ) );
        break;
    }
    case VTK_QUADRATIC_HEXAHEDRON: {
        constexpr int order[] = { 4, 5, 6, 7, 0, 1, 2, 3, 12, 13, 14, 15, 8, 9, 10, 11 };
        ::Reorder( kvs_connection, vtk_cell, order, sizeof( order ) / sizeof( int ) );
        break;
    }
    case VTK_VERTEX: {
        constexpr int order[] = { 0 };
        ::Reorder( kvs_connection, vtk_cell, order, sizeof( order ) / sizeof( int ) );
        break;
    }
    case VTK_PYRAMID: {
        constexpr int order[] = { 4, 0, 1, 2, 3 };
        ::Reorder( kvs_connection, vtk_cell, order, sizeof( order ) / sizeof( int ) );
        break;
    }
    case VTK_WEDGE: {
        constexpr int order[] = { 0, 1, 2, 3, 4, 5 };
        ::Reorder( kvs_connection, vtk_cell, order, sizeof( order ) / sizeof( int ) );
        break;
    }
    default:
        throw std::runtime_error( "Unsupported VTK cell type" );
    }
}
} // namespace


void cvt::detail::ImportPolygonObject( kvs::PolygonObject* polygon_object,
                                       vtkSmartPointer<vtkPolyData> data )
{
    auto coords = ::GetCoordinates( data );
    polygon_object->setCoords( coords );

    // Normals
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
        normals_filter->AutoOrientNormalsOn();
        normals_filter->ComputeCellNormalsOn();
        normals_filter->ComputePointNormalsOff();
        normals_filter->SetOutputPointsPrecision(
            vtkAlgorithm::DesiredOutputPrecision::SINGLE_PRECISION );

        normals_filter->Update();

        stl_normals = normals_filter->GetOutput()->GetCellData()->GetArray( "Normals" );
        data->GetCellData()->AddArray( stl_normals );

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

    // Connectivity
    auto cell_array = data->GetPolys();
    std::vector<kvs::UInt32> connections( cell_array->GetNumberOfConnectivityIds() );

    bool is_triangle_mesh = data->GetCell( 0 ) && data->GetCell( 0 )->GetCellType() == VTK_TRIANGLE;
    auto head = connections.begin();
    vtkNew<vtkIdList> cell_ids;
    for ( vtkIdType i = 0; i < data->GetNumberOfCells(); ++i )
    {
        auto cell = data->GetCell( i );

        if ( is_triangle_mesh )
        {
            if ( cell->GetCellType() == VTK_TRIANGLE )
            {
                data->GetCellPoints( i, cell_ids );
                ::ReorderElementNodeIndices( head, cell_ids, VTK_TRIANGLE );
            }
        }
        else
        {
            if ( cell->GetCellType() == VTK_QUAD || cell->GetCellType() == VTK_PIXEL )
            {
                data->GetCellPoints( i, cell_ids );
                ::ReorderElementNodeIndices( head, cell_ids, cell->GetCellType() );
            }
        }
    }
    connections.resize( std::distance( connections.begin(), head ) );
    auto v_connections = kvs::ValueArray<kvs::UInt32>( connections );

    polygon_object->setPolygonType( ( is_triangle_mesh )
                                        ? kvs::PolygonObject::PolygonType::Triangle
                                        : kvs::PolygonObject::PolygonType::Quadrangle );
    polygon_object->setNormalType( kvs::PolygonObject::PolygonNormal );
    polygon_object->setNormals( v_normals );
    polygon_object->setConnections( v_connections );
    polygon_object->updateMinMaxCoords();
}

void cvt::detail::ImportPolygonObject( kvs::PolygonObject* polygon_object,
                                       vtkSmartPointer<vtkUnstructuredGrid> data )
{
    auto polydata = vtkSmartPointer<vtkPolyData>::New();
    polydata->SetPoints( data->GetPoints() );
    polydata->SetPolys( data->GetCells() );

    cvt::detail::ImportPolygonObject( polygon_object, polydata );
}

void cvt::detail::ImportRectilinearObject( kvs::StructuredVolumeObject* rectilinear_object,
                                           vtkSmartPointer<vtkRectilinearGrid> input )
{
    vtkNew<vtkCellDataToPointData> cell_data_to_point_data;
    cell_data_to_point_data->SetInputData( input );
    cell_data_to_point_data->Update();

    vtkSmartPointer<vtkRectilinearGrid> data =
        dynamic_cast<vtkRectilinearGrid*>( cell_data_to_point_data->GetOutput() );

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
                                                       vtkSmartPointer<vtkImageData> input )
{
    vtkNew<vtkCellDataToPointData> cell_data_to_point_data;
    cell_data_to_point_data->SetInputData( input );
    cell_data_to_point_data->Update();

    vtkSmartPointer<vtkImageData> data =
        dynamic_cast<vtkImageData*>( cell_data_to_point_data->GetOutput() );

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

void cvt::detail::ImportIrregularStructuredVolumeObject(
    kvs::StructuredVolumeObject* irregular_object, vtkSmartPointer<vtkStructuredGrid> input )
{
    vtkNew<vtkCellDataToPointData> cell_data_to_point_data;
    cell_data_to_point_data->SetInputData( input );
    cell_data_to_point_data->Update();

    vtkSmartPointer<vtkStructuredGrid> data =
        dynamic_cast<vtkStructuredGrid*>( cell_data_to_point_data->GetOutput() );

    auto bounding_box = data->GetBounds();
    kvs::Vector3f min_obj_coord( bounding_box[0], bounding_box[2], bounding_box[4] );
    kvs::Vector3f max_obj_coord( bounding_box[1], bounding_box[3], bounding_box[5] );
    irregular_object->setMinMaxObjectCoords( min_obj_coord, max_obj_coord );

    irregular_object->setGridType( kvs::StructuredVolumeObject::Curvilinear );
    irregular_object->setResolution( ::GetResolution( data ) );

    auto v_coords = GetCoordinates( data );
    irregular_object->setCoords( v_coords );

    auto component_count = ::GetCountOfComponents( data );
    irregular_object->setVeclen( component_count );
    if ( component_count != 0 )
    {
        auto values = ::GetValueArray( data, component_count );
        irregular_object->setValues( values );
        irregular_object->updateMinMaxValues();
    }
}

void cvt::detail::ImportUnstructuredVolumeObject( kvs::UnstructuredVolumeObject* object,
                                                  vtkSmartPointer<vtkUnstructuredGrid> input )
{
    vtkNew<vtkCellDataToPointData> cell_data_to_point_data;
    cell_data_to_point_data->SetInputData( input );
    cell_data_to_point_data->Update();

    vtkSmartPointer<vtkUnstructuredGrid> data =
        dynamic_cast<vtkUnstructuredGrid*>( cell_data_to_point_data->GetOutput() );

    object->setNumberOfNodes( data->GetNumberOfPoints() );
    object->setNumberOfCells( data->GetNumberOfCells() );

    auto first_cell = data->GetCell( 0 );
    auto cell_type = first_cell->GetCellType();
    object->setCellType( ::GetKvsCellType( cell_type ) );

    auto coords = GetCoordinates( data );
    object->setCoords( coords );

    auto cell_point_count = first_cell->GetNumberOfPoints();
    kvs::UnstructuredVolumeObject::Connections connections( cell_point_count *
                                                            data->GetNumberOfCells() );

    auto head = connections.begin();
    for ( vtkIdType i = 0; i < data->GetNumberOfCells(); ++i )
    {
        auto cell = data->GetCell( i );

        if ( cell->GetCellType() != cell_type )
        {
            throw std::runtime_error( "hetero cell types" );
        }

        ::ReorderElementNodeIndices( head, cell->GetPointIds(), cell->GetCellType() );
    }
    object->setConnections( connections );

    auto component_count = ::GetCountOfComponents( data );

    object->setVeclen( component_count );
    if ( component_count != 0 )
    {
        auto values = ::GetValueArray( data, component_count );
        object->setValues( values );
        object->updateMinMaxValues();
    }

    object->updateMinMaxCoords();
}

void cvt::detail::ImportPointObject( kvs::PointObject* object,
                                     vtkSmartPointer<vtkUnstructuredGrid> data )
{
    auto coords = GetCoordinates( data );
    object->setCoords( coords );

    std::vector<kvs::Real32> normals( data->GetNumberOfPoints() * 3, 0 );
    for ( int i = 0; i < data->GetNumberOfPoints(); ++i )
    {
        normals[i * 3] = 1.0;
    }
    auto v_normals = kvs::ValueArray<kvs::Real32>( normals );
    object->setNormals( v_normals );

    object->updateMinMaxCoords();
}

void cvt::detail::ImportLineObject( kvs::LineObject* object,
                                    vtkSmartPointer<vtkUnstructuredGrid> data )
{
    object->setLineType( kvs::LineObject::Segment );

    auto coords = GetCoordinates( data );
    object->setCoords( coords );

    std::deque<kvs::UInt32> connection_deque;
    for ( vtkIdType i = 0; i < data->GetNumberOfCells(); ++i )
    {
        auto cell = data->GetCell( i );

        switch ( cell->GetCellType() )
        {
        case VTK_LINE: {
            connection_deque.push_back( static_cast<kvs::UInt32>( cell->GetPointId( 0 ) ) );
            connection_deque.push_back( static_cast<kvs::UInt32>( cell->GetPointId( 1 ) ) );
            break;
        }
        case VTK_POLY_LINE: {
            auto ids = cell->GetPointIds();
            for ( int j = 0; j < ids->GetNumberOfIds() - 1; ++j )
            {
                connection_deque.push_back( static_cast<kvs::UInt32>( cell->GetPointId( j ) ) );
                connection_deque.push_back( static_cast<kvs::UInt32>( cell->GetPointId( j + 1 ) ) );
            }
            break;
        }
        }
    }
    kvs::ValueArray<kvs::UInt32> connections( connection_deque.size() );
    std::copy( connection_deque.begin(), connection_deque.end(), connections.begin() );
    object->setConnections( connections );

    object->updateMinMaxCoords();
}
