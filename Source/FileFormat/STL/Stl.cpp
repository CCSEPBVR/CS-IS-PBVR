/*----------------------------------------------------------------------------
 *
 *  Copyright (c) Visualization Laboratory, Kyoto University.
 *  All rights reserved.
 *  See http://www.viz.media.kyoto-u.ac.jp/kvs/copyright/ for details.
 */
#include "FileFormat/STL/Stl.h"

#include <algorithm>
#include <cstring>
#include <exception>
#include <locale>
#include <string>

#include "kvs/File"
#include "kvs/PolygonObject"
#include "kvs/Type"
#include "kvs/ValueArray"

#include <vtkCellArray.h>
#include <vtkCellData.h>
#include <vtkDataArray.h>
#include <vtkNew.h>
#include <vtkPointData.h>
#include <vtkPoints.h>
#include <vtkPolyData.h>
#include <vtkPolyDataNormals.h>
#include <vtkSTLReader.h>
#include <vtkSTLWriter.h>
#include <vtkSmartPointer.h>

namespace
{

void ConvertToPolygonObject( kvs::PolygonObject* polygon_object, vtkSmartPointer<vtkPolyData> data )
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
} // namespace

bool cvt::Stl::CheckExtension( const std::string& filename )
{
    const kvs::File file( filename );
    auto extension = file.extension();

    std::transform( extension.cbegin(), extension.cend(), extension.begin(), tolower );

    return extension == "stl" || extension == "stla" || extension == "stlb" || extension == "sla" ||
           extension == "slb";
}

bool cvt::Stl::read( const std::string& filename )
{
    BaseClass::setFilename( filename );
    BaseClass::setSuccess( false );

    try
    {
        vtkNew<vtkSTLReader> reader;
        reader->SetFileName( filename.c_str() );
        reader->Update();

        polydata = reader->GetOutput();

        bool is_success = polydata->GetNumberOfCells() > 0;
        BaseClass::setSuccess( is_success );

        return is_success;
    }
    catch ( std::exception& e )
    {
        throw e;
    }
    catch ( ... )
    {
        return false;
    }
}

bool cvt::Stl::write( const std::string& filename )
{
    BaseClass::setFilename( filename );
    BaseClass::setSuccess( false );

    try
    {
        vtkNew<vtkSTLWriter> writer;
        writer->SetInputData( polydata );
        writer->SetFileName( filename.c_str() );

        bool is_success = writer->Write() == 1;
        BaseClass::setSuccess( is_success );

        return is_success;
    }
    catch ( std::exception& e )
    {
        throw e;
    }
    catch ( ... )
    {
        return false;
    }
}

void cvt::Stl::get( kvs::PolygonObject* object ) { ConvertToPolygonObject( object, polydata ); }

void cvt::Stl::get( kvs::PolygonObject* object ) const
{
    ConvertToPolygonObject( object, polydata );
}

void cvt::Stl::set( const kvs::PolygonObject& object )
{
    if ( object.polygonType() != kvs::PolygonObject::Triangle )
    {
        throw std::runtime_error( "Input object is not triangle polygon" );
    }

    vtkNew<vtkPolyData> p;

    vtkNew<vtkPoints> points;
    p->SetPoints( points );
    points->SetDataTypeToFloat();
    auto coords = object.coords().data();
    auto point_count = object.numberOfVertices();
    points->SetNumberOfPoints( static_cast<vtkIdType>( point_count ) );

    for ( vtkIdType i = 0; i < point_count; ++i )
    {
        points->SetPoint( i, *( coords + i * 3 ), *( coords + i * 3 + 1 ),
                          *( coords + i * 3 + 2 ) );
    }

    if ( object.numberOfConnections() == 0 )
    {
        vtkNew<vtkCellArray> cell_array;

        for ( vtkIdType i = 0; i < object.numberOfVertices() / 3; ++i )
        {
            vtkIdType ids[3] = { i * 3, i * 3 + 1, i * 3 + 2 };
            cell_array->InsertNextCell( 3, ids );
        }

        p->SetPolys( cell_array );
    }
    else
    {
        auto cell_count = object.connections().size() / 3;
        throw std::runtime_error( "not implemented" );
    }

    polydata = p;
}