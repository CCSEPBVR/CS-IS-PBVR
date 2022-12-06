#include "UnstructuredPfi.h"

#include <algorithm>
#include <cstdio>
#include <limits>
#include <string>
#include <vector>

#include "kvs/KVSMLUnstructuredVolumeObject"
#include "kvs/UnstructuredVolumeObject"
#include "kvs/Vector3"

namespace
{

template <typename T>
std::vector<std::vector<T>> Allocate2DVector( int x, int y )
{
    std::vector<std::vector<T>> v( x );

    for ( int i = 0; i < x; ++i )
    {
        v[i].resize( y );
    }

    return v;
}

int GetCellId( const std::string& cell_type_expr )
{
    if ( cell_type_expr == "Unknown cell type" )
    {
        return kvs::UnstructuredVolumeObject::UnknownCellType;
    }
    else if ( cell_type_expr == "tetrahedra" )
    {
        return kvs::UnstructuredVolumeObject::Tetrahedra;
    }
    else if ( cell_type_expr == "quadratic tetrahedra" )
    {
        return kvs::UnstructuredVolumeObject::QuadraticTetrahedra;
    }
    else if ( cell_type_expr == "hexahedra" )
    {
        return kvs::UnstructuredVolumeObject::Hexahedra;
    }
    else if ( cell_type_expr == "quadratic hexahedra" )
    {
        return kvs::UnstructuredVolumeObject::QuadraticHexahedra;
    }
    else if ( cell_type_expr == "pyramid" )
    {
        return kvs::UnstructuredVolumeObject::Pyramid;
    }
    else if ( cell_type_expr == "prism" )
    {
        return kvs::UnstructuredVolumeObject::Prism;
    }
    else if ( cell_type_expr == "point" )
    {
        return kvs::UnstructuredVolumeObject::Point;
    }
    else
    {
        throw std::runtime_error( "Not supported cell type." );
    }

    return -1;
}

template <typename A, typename B>
void UpdateMinMaxValues( A& mins, A& maxes, const B& array, int veclen, int number_of_nodes )
{
    for ( int v = 0; v < veclen; ++v )
    {
        float min = std::numeric_limits<float>::max();
        float max = std::numeric_limits<float>::min();

        for ( int i = v * number_of_nodes; i < ( v + 1 ) * number_of_nodes; ++i )
        {
            min = std::min( min, static_cast<float>( array[i] ) );
            max = std::max( max, static_cast<float>( array[i] ) );
        }

        mins[v] = std::min( min, mins[v] );
        maxes[v] = std::max( max, maxes[v] );
    }
}

template <typename T, typename Array, typename FileDescriptor>
void WriteN( Array& array, FileDescriptor f, std::size_t n, std::size_t start = 0 )
{
    for ( std::size_t i = start; i < start + n; ++i )
    {
        T m = array[i];
        std::fwrite( &m, sizeof( T ), 1, f );
    }
}
} // namespace

cvt::UnstructuredPfi::UnstructuredPfi( int number_of_components, int last_time_step,
                                       int max_sub_volume_id ):
    number_of_component( number_of_components ),
    max_sub_volume_id( max_sub_volume_id ),
    last_time_step( last_time_step )
{
    if ( last_time_step < 0 )
    {
        throw std::runtime_error( "The last time step must be 0 or a positive number" );
    }
    if ( max_sub_volume_id < 1 )
    {
        throw std::runtime_error( "The max sub volume ID must be a positive number" );
    }

    int t = last_time_step + 1;
    int s = max_sub_volume_id;

    node_counts = ::Allocate2DVector<int>( t, s );
    element_counts = ::Allocate2DVector<int>( t, s );

    min_external_coords = ::Allocate2DVector<kvs::Vec3>( t, s );
    max_external_coords = ::Allocate2DVector<kvs::Vec3>( t, s );
    min_object_coords = ::Allocate2DVector<kvs::Vec3>( t, s );
    max_object_coords = ::Allocate2DVector<kvs::Vec3>( t, s );
    min_values = ::Allocate2DVector<float>( t, number_of_component );
    max_values = ::Allocate2DVector<float>( t, number_of_component );

    for ( int i = 0; i < t; ++i )
    {
        std::fill( min_values[i].begin(), min_values[i].end(), std::numeric_limits<float>::max() );
        std::fill( max_values[i].begin(), max_values[i].end(), std::numeric_limits<float>::min() );
    }
}

void cvt::UnstructuredPfi::registerObject( kvs::KVSMLUnstructuredVolumeObject* object,
                                           int time_step, int sub_volume_id )
{
    int sub_volume = sub_volume_id - 1;
    node_counts[time_step][sub_volume] = static_cast<int>( object->nnodes() );
    element_counts[time_step][sub_volume] = static_cast<int>( object->ncells() );

    type_of_elements = ::GetCellId( object->cellType() );

    min_external_coords[time_step][sub_volume] = object->minExternalCoord();
    max_external_coords[time_step][sub_volume] = object->maxExternalCoord();

    min_object_coords[time_step][sub_volume] = object->minObjectCoord();
    max_external_coords[time_step][sub_volume] = object->maxObjectCoord();

    auto values = object->values().asValueArray<float>();
    ::UpdateMinMaxValues( min_values[time_step], max_values[time_step], values, number_of_component,
                          object->nnodes() );
}

bool cvt::UnstructuredPfi::write( const char* const filename )
{
    if ( auto f = fopen( filename, "wb" ) )
    {
        int number_of_nodes = std::accumulate( node_counts[0].begin(), node_counts[0].end(), 0 );
        std::fwrite( &number_of_nodes, sizeof( int ), 1, f );

        int number_of_elements =
            std::accumulate( element_counts[0].begin(), element_counts[0].end(), 0 );
        std::fwrite( &number_of_elements, sizeof( int ), 1, f );

        std::fwrite( &type_of_elements, sizeof( int ), 1, f );

        int type_of_file = 0;
        std::fwrite( &type_of_file, sizeof( int ), 1, f );

        int number_of_file = max_sub_volume_id * ( last_time_step + 1 );
        std::fwrite( &number_of_file, sizeof( int ), 1, f );

        std::fwrite( &number_of_component, sizeof( int ), 1, f );

        int step_of_beginning = 0;
        std::fwrite( &step_of_beginning, sizeof( int ), 1, f );
        int step_of_end = 0;
        std::fwrite( &step_of_end, sizeof( int ), 1, f );

        int number_of_sub_volumes = max_sub_volume_id;
        std::fwrite( &number_of_sub_volumes, sizeof( int ), 1, f );

        for ( int i = 0; i < max_sub_volume_id; ++i )
        {
            ::WriteN<float>( min_external_coords[0][i], f, 3 );
            ::WriteN<float>( max_external_coords[0][i], f, 3 );
        }

        for ( int i = 0; i < max_sub_volume_id; ++i )
        {
            std::fwrite( &node_counts[0][i], sizeof( int ), 1, f );
        }
        for ( int i = 0; i < max_sub_volume_id; ++i )
        {
            std::fwrite( &element_counts[0][i], sizeof( int ), 1, f );
        }

        for ( int i = 0; i <= last_time_step; ++i )
        {
            ::WriteN<float>( min_object_coords[0][i], f, 3 );
            ::WriteN<float>( max_object_coords[0][i], f, 3 );
        }

        for ( int i = 0; i <= last_time_step; ++i )
        {
            for ( int j = 0; j < number_of_component; ++j )
            {

                std::fwrite( &min_values[i][j], sizeof( float ), 1, f );
                std::fwrite( &max_values[i][j], sizeof( float ), 1, f );
            }
        }

        std::fclose( f );

        return true;
    }
    else
    {
        return false;
    }
}