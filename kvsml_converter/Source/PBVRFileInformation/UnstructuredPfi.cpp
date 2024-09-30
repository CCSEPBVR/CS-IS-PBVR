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

template <typename A>
void UpdateMinMaxValues( A& mins, A& maxes, const kvs::AnyValueArray& array, int veclen,
                         int number_of_nodes )
{
    auto minmax = cvt::detail::GetMinMaxValues( array, veclen, number_of_nodes );
    for ( int v = 0; v < veclen; ++v )
    {
        mins[v] = std::min( mins[v], std::get<0>( minmax )[v] );
        maxes[v] = std::max( maxes[v], std::get<1>( minmax )[v] );
    }
}

template <typename T, typename Array, typename FileDescriptor>
void WriteN( Array& array, FileDescriptor f, std::size_t n, std::string name , std::size_t start = 0 )
{
    for ( std::size_t i = start; i < start + n; ++i )
    {
        T m = array[i];
        //std::fwrite( &m, sizeof( T ), 1, f );
        std::string m_str = name + "_" +  std::to_string(i) +  "=" + std::to_string(float(m)) + "\n";
        std::fwrite( m_str.c_str(), sizeof( char ), m_str.size(), f );
    }
}

//template <typename T, typename Array, typename FileDescriptor>
//void WriteN( Array& array, FileDescriptor f, std::size_t n, std::size_t start = 0 )
//{
//    for ( std::size_t i = start; i < start + n; ++i )
//    {
//        T m = array[i];
//        std::fwrite( &m, sizeof( T ), 1, f );
//    }
//}
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
    cell_counts = ::Allocate2DVector<int>( t, s );

    min_external_coords = kvs::Vec3::Constant( std::numeric_limits<float>::max() );
    max_external_coords = kvs::Vec3::Constant( std::numeric_limits<float>::lowest() );
    min_object_coords = ::Allocate2DVector<kvs::Vec3>( t, s );
    max_object_coords = ::Allocate2DVector<kvs::Vec3>( t, s );
    min_values = ::Allocate2DVector<float>( t, number_of_component );
    max_values = ::Allocate2DVector<float>( t, number_of_component );

    for ( int i = 0; i < t; ++i )
    {
        std::fill( min_values[i].begin(), min_values[i].end(), std::numeric_limits<float>::max() );
        std::fill( max_values[i].begin(), max_values[i].end(),
                   std::numeric_limits<float>::lowest() );
    }
}

void cvt::UnstructuredPfi::registerObject( kvs::KVSMLUnstructuredVolumeObject* object,
                                           int time_step, int sub_volume_id )
{
    int sub_volume = sub_volume_id - 1;
    node_counts[time_step][sub_volume] = static_cast<int>( object->nnodes() );
    cell_counts[time_step][sub_volume] = static_cast<int>( object->ncells() );

    type_of_cells = cvt::detail::GetKvsCellTypeId( object->cellType() );

    cvt::detail::MinVec3( min_external_coords, object->minExternalCoord() );
    cvt::detail::MaxVec3( max_external_coords, object->maxExternalCoord() );

    min_object_coords[time_step][sub_volume] = object->minObjectCoord();
    max_object_coords[time_step][sub_volume] = object->maxObjectCoord();

    ::UpdateMinMaxValues( min_values[time_step], max_values[time_step], object->values(),
                          number_of_component, object->nnodes() );
}

void cvt::UnstructuredPfi::registerObject( kvs::KVSMLStructuredVolumeObject* object, int time_step,
                                           int sub_volume_id )
{
    int sub_volume = sub_volume_id - 1;
    auto resolution = object->resolution();

    auto number_of_nodes = static_cast<int>( resolution[0] * resolution[1] * resolution[2] );
    node_counts[time_step][sub_volume] = number_of_nodes;
    cell_counts[time_step][sub_volume] =
        static_cast<int>( ( resolution[0] - 1 ) * ( resolution[1] - 1 ) * ( resolution[2] - 1 ) );

    type_of_cells = 7;

    cvt::detail::MinVec3( min_external_coords, object->minExternalCoord() );
    cvt::detail::MaxVec3( max_external_coords, object->maxExternalCoord() );

    min_object_coords[time_step][sub_volume] = object->minObjectCoord();
    max_object_coords[time_step][sub_volume] = object->maxObjectCoord();

    ::UpdateMinMaxValues( min_values[time_step], max_values[time_step], object->values(),
                          number_of_component, number_of_nodes );
}

void cvt::UnstructuredPfi::registerObject( const ConverterTaskOutput* output,
                                           const std::string& key )
{
    for ( int sub_volume_id = 0; sub_volume_id < output->sub_volume_counts.at( key );
          ++sub_volume_id )
    {
        int time_step = output->time_step;

        node_counts[time_step][sub_volume_id] = output->node_counts.at( key )[sub_volume_id];
        cell_counts[time_step][sub_volume_id] = output->cell_counts.at( key )[sub_volume_id];

        type_of_cells = output->type_of_cells.at( key );

        cvt::detail::MinVec3( min_external_coords, output->min_external_coords );
        cvt::detail::MaxVec3( max_external_coords, output->max_external_coords );

        min_object_coords[time_step][sub_volume_id] =
            output->min_object_coords.at( key )[sub_volume_id];
        max_object_coords[time_step][sub_volume_id] =
            output->max_object_coords.at( key )[sub_volume_id];

        auto& mins = std::get<0>( output->value_minmax.at( key ) );
        auto& maxes = std::get<1>( output->value_minmax.at( key ) );
        for ( int v = 0; v < number_of_component; ++v )
        {
            min_values[time_step][v] = std::min( min_values[time_step][v], mins[v] );
            max_values[time_step][v] = std::max( max_values[time_step][v], maxes[v] );
        }
    }
}

bool cvt::UnstructuredPfi::write( const char* const filename )
{
    if ( auto f = fopen( filename, "w" ) )
    {
        int number_of_nodes = std::accumulate( node_counts[0].begin(), node_counts[0].end(), 0 );
        std::string number_of_nodes_str = "NODE_NUM=" + std::to_string(number_of_nodes) + "\n";
        std::fwrite( number_of_nodes_str.c_str(), sizeof( char ), number_of_nodes_str.size(), f );
        
        int number_of_elements =
            std::accumulate( cell_counts[0].begin(), cell_counts[0].end(), 0 );
        std::string number_of_elements_str = "CELL_NUM=" + std::to_string(number_of_elements) + "\n";
        std::fwrite( number_of_elements_str.c_str(), sizeof( char ), number_of_elements_str.size(), f );

        std::string type_of_elements_str  = "CELLTYPE=" + std::to_string(type_of_cells) + "\n";
        std::fwrite( type_of_elements_str.c_str(), sizeof( char ), type_of_elements_str.size(), f );

        int type_of_file = 0;
        std::string type_of_file_str  = "FILETYPE=" +std::to_string(type_of_file) + "\n" ;
        std::fwrite( type_of_file_str.c_str(), sizeof( char ), type_of_file_str.size(), f );

        int number_of_file = max_sub_volume_id * ( last_time_step + 1 );
        std::string number_of_file_str = "FILES_NUM=" + std::to_string(number_of_file) + "\n";
        std::fwrite( number_of_file_str.c_str(), sizeof( char ), number_of_file_str.size(), f );

        std::string number_of_component_str = "COMPONENT_NUM=" + std::to_string(number_of_component) + "\n";
        std::fwrite( number_of_component_str.c_str(), sizeof( char ), number_of_component_str.size(), f );

        int step_of_beginning = 0;
        std::string step_of_beginning_str = "BEGINNING_STEP=" + std::to_string(step_of_beginning) + "\n";
        std::fwrite( step_of_beginning_str.c_str(), sizeof( char ), step_of_beginning_str.size(), f );
        int step_of_end = last_time_step;
        std::string step_of_end_str = "LAST_STEP=" + std::to_string(step_of_end) + "\n";
        std::fwrite( step_of_end_str.c_str(), sizeof( char ), step_of_end_str.size(), f );

        int number_of_sub_volumes = max_sub_volume_id;
        std::string number_of_sub_volumes_str = "SUBVOLUME_NUM=" + std::to_string(number_of_sub_volumes) + "\n";
        std::fwrite( number_of_sub_volumes_str.c_str(), sizeof( char ), number_of_sub_volumes_str.size(), f );

        std::string name = "MIN_COORD" ;
        ::WriteN<float>( min_external_coords, f, 3, name);
        name = "MAX_COORD" ;
        ::WriteN<float>( max_external_coords, f, 3, name );

        for ( int i = 0; i < max_sub_volume_id; ++i )
        {
        std::string node_counts_str = "SUBVOLUME_NODE_NUM_" + std::to_string(i) +"=" + std::to_string(node_counts[0][i]) + "\n";
        std::fwrite( node_counts_str.c_str(), sizeof( char ), node_counts_str.size(), f );
        }
        for ( int i = 0; i < max_sub_volume_id; ++i )
        {
        std::string element_counts_str = "ELEMENT_COUNT_NUM_" + std::to_string(i) +  "=" + std::to_string(cell_counts[0][i]) + "\n";
        std::fwrite( element_counts_str.c_str(), sizeof( char ), element_counts_str.size(), f );
        }

        for ( int i = 0; i < max_sub_volume_id; ++i )
        {
            std::string name = "SUB_VOLUME_MIN_COORD_" + std::to_string(i);
            ::WriteN<float>( min_object_coords[0][i], f, 3 ,name );
            name = "SUB_VOLUME_MAX_COORD_" + std::to_string(i) ;
            ::WriteN<float>( max_object_coords[0][i], f, 3 ,name );
        }

        std::fclose( f );

        return true;
    }
    else
    {
        return false;
    }

#if 0
    if ( auto f = fopen( filename, "wb" ) )
    {
        int number_of_nodes = std::accumulate( node_counts[0].begin(), node_counts[0].end(), 0 );
        std::fwrite( &number_of_nodes, sizeof( int ), 1, f );

        int number_of_elements = std::accumulate( cell_counts[0].begin(), cell_counts[0].end(), 0 );
        std::fwrite( &number_of_elements, sizeof( int ), 1, f );

        std::fwrite( &type_of_cells, sizeof( int ), 1, f );

        int type_of_file = 0;
        std::fwrite( &type_of_file, sizeof( int ), 1, f );

        int number_of_file = max_sub_volume_id * ( last_time_step + 1 );
        std::fwrite( &number_of_file, sizeof( int ), 1, f );

        std::fwrite( &number_of_component, sizeof( int ), 1, f );

        int step_of_beginning = 0;
        std::fwrite( &step_of_beginning, sizeof( int ), 1, f );
        int step_of_end = last_time_step;
        std::fwrite( &step_of_end, sizeof( int ), 1, f );

        int number_of_sub_volumes = max_sub_volume_id;
        std::fwrite( &number_of_sub_volumes, sizeof( int ), 1, f );

        ::WriteN<float>( min_external_coords, f, 3 );
        ::WriteN<float>( max_external_coords, f, 3 );

        for ( int i = 0; i < max_sub_volume_id; ++i )
        {
            std::fwrite( &node_counts[0][i], sizeof( int ), 1, f );
        }
        for ( int i = 0; i < max_sub_volume_id; ++i )
        {
            std::fwrite( &cell_counts[0][i], sizeof( int ), 1, f );
        }

        ::WriteN<float>( min_object_coords[0][0], f, 3 );
        ::WriteN<float>( max_object_coords[0][0], f, 3 );

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
#endif
}

int cvt::detail::GetKvsCellTypeId( const std::string& cell_type_expr )
{
    if ( cell_type_expr == "tetrahedra" )
    {
        return 4;
    }
    else if ( cell_type_expr == "quadratic tetrahedra" )
    {
        return 11;
    }
    else if ( cell_type_expr == "hexahedra" )
    {
        return 7;
    }
    else if ( cell_type_expr == "quadratic hexahedra" )
    {
        return 14;
    }
    else if ( cell_type_expr == "pyramid" )
    {
        return 5;
    }
    else if ( cell_type_expr == "prism" )
    {
        return 6;
    }
    else
    {
        throw std::runtime_error( "Not supported cell type." );
    }

    return -1;
}
