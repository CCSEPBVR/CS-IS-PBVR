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

int GetCellId( const std::string& cell_type_expr )
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

template <typename A, typename B>
void UpdateMinMaxValuesImpl( A& mins, A& maxes, const B& array, int veclen, int number_of_nodes )
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

template <typename A, typename B>
void UpdateMinMaxValues( A& mins, A& maxes, const B& array, int veclen, int number_of_nodes )
{
    switch ( array.typeID() )
    {
    case kvs::Type::TypeID::TypeReal32: {
        auto values = array.template asValueArray<kvs::Real32>();
        ::UpdateMinMaxValuesImpl( mins, maxes, values, veclen, number_of_nodes );
        break;
    }
    case kvs::Type::TypeID::TypeInt8: {
        auto values = array.template asValueArray<kvs::Int8>();
        ::UpdateMinMaxValuesImpl( mins, maxes, values, veclen, number_of_nodes );
        break;
    }
    case kvs::Type::TypeID::TypeInt16: {
        auto values = array.template asValueArray<kvs::Int16>();
        ::UpdateMinMaxValuesImpl( mins, maxes, values, veclen, number_of_nodes );
        break;
    }
    case kvs::Type::TypeID::TypeInt32: {
        auto values = array.template asValueArray<kvs::Int32>();
        ::UpdateMinMaxValuesImpl( mins, maxes, values, veclen, number_of_nodes );
        break;
    }
    case kvs::Type::TypeID::TypeInt64: {
        auto values = array.template asValueArray<kvs::Int64>();
        ::UpdateMinMaxValuesImpl( mins, maxes, values, veclen, number_of_nodes );
        break;
    }
    case kvs::Type::TypeID::TypeUInt8: {
        auto values = array.template asValueArray<kvs::UInt8>();
        ::UpdateMinMaxValuesImpl( mins, maxes, values, veclen, number_of_nodes );
        break;
    }
    case kvs::Type::TypeID::TypeUInt16: {
        auto values = array.template asValueArray<kvs::UInt16>();
        ::UpdateMinMaxValuesImpl( mins, maxes, values, veclen, number_of_nodes );
        break;
    }
    case kvs::Type::TypeID::TypeUInt32: {
        auto values = array.template asValueArray<kvs::UInt32>();
        ::UpdateMinMaxValuesImpl( mins, maxes, values, veclen, number_of_nodes );
        break;
    }
    case kvs::Type::TypeID::TypeUInt64: {
        auto values = array.template asValueArray<kvs::UInt64>();
        ::UpdateMinMaxValuesImpl( mins, maxes, values, veclen, number_of_nodes );
        break;
    }
    case kvs::Type::TypeID::TypeReal64:
    default: {
        auto values = array.template asValueArray<kvs::Real64>();
        ::UpdateMinMaxValuesImpl( mins, maxes, values, veclen, number_of_nodes );
    }
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
    element_counts[time_step][sub_volume] =
        static_cast<int>( ( resolution[0] - 1 ) * ( resolution[1] - 1 ) * ( resolution[2] - 1 ) );

    type_of_elements = 7;

    min_external_coords[time_step][sub_volume] = object->minExternalCoord();
    max_external_coords[time_step][sub_volume] = object->maxExternalCoord();

    min_object_coords[time_step][sub_volume] = object->minObjectCoord();
    max_object_coords[time_step][sub_volume] = object->maxObjectCoord();

    ::UpdateMinMaxValues( min_values[time_step], max_values[time_step], object->values(),
                          number_of_component, number_of_nodes );
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
        int step_of_end = last_time_step;
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