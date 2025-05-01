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
#ifndef CVT__UNSTRUCTURED_PFI_H_INCLUDE
#define CVT__UNSTRUCTURED_PFI_H_INCLUDE

#include <limits>
#include <string>
#include <vector>

#include "kvs/AnyValueArray"
#include "kvs/KVSMLStructuredVolumeObject"
#include "kvs/KVSMLUnstructuredVolumeObject"
#include "kvs/Message"
#include "kvs/Vector3"

#include "Converter/ConverterTaskOutput.h"
#include "Filesystem.h"

namespace cvt
{
namespace detail
{
template <typename Array>
inline std::tuple<std::vector<float>, std::vector<float>> GetMinMaxValuesImpl( const Array& array,
                                                                               int veclen,
                                                                               int number_of_nodes )
{
    std::tuple<std::vector<float>, std::vector<float>> minmax(
        std::vector<float>( veclen, std::numeric_limits<float>::max() ),
        std::vector<float>( veclen, std::numeric_limits<float>::lowest() ) );
    auto& mins = std::get<0>( minmax );
    auto& maxes = std::get<1>( minmax );

    for ( int v = 0; v < veclen; ++v )
    {
        auto min = static_cast<float>( *std::min_element(
            array.data() + v * number_of_nodes, array.data() + ( v + 1 ) * number_of_nodes ) );
        auto max = static_cast<float>( *std::max_element(
            array.data() + v * number_of_nodes, array.data() + ( v + 1 ) * number_of_nodes ) );

        mins[v] = std::min( min, mins[v] );
        maxes[v] = std::max( max, maxes[v] );
    }

    return minmax;
}

inline std::tuple<std::vector<float>, std::vector<float>> GetMinMaxValues(
    const kvs::AnyValueArray& array, int veclen, int number_of_nodes )
{
    switch ( array.typeID() )
    {
    case kvs::Type::TypeID::TypeReal32: {
        auto values = array.template asValueArray<kvs::Real32>();
        return cvt::detail::GetMinMaxValuesImpl( values, veclen, number_of_nodes );
    }
    case kvs::Type::TypeID::TypeInt8: {
        auto values = array.template asValueArray<kvs::Int8>();
        return cvt::detail::GetMinMaxValuesImpl( values, veclen, number_of_nodes );
    }
    case kvs::Type::TypeID::TypeInt16: {
        auto values = array.template asValueArray<kvs::Int16>();
        return cvt::detail::GetMinMaxValuesImpl( values, veclen, number_of_nodes );
    }
    case kvs::Type::TypeID::TypeInt32: {
        auto values = array.template asValueArray<kvs::Int32>();
        return cvt::detail::GetMinMaxValuesImpl( values, veclen, number_of_nodes );
    }
    case kvs::Type::TypeID::TypeInt64: {
        auto values = array.template asValueArray<kvs::Int64>();
        return cvt::detail::GetMinMaxValuesImpl( values, veclen, number_of_nodes );
    }
    case kvs::Type::TypeID::TypeUInt8: {
        auto values = array.template asValueArray<kvs::UInt8>();
        return cvt::detail::GetMinMaxValuesImpl( values, veclen, number_of_nodes );
    }
    case kvs::Type::TypeID::TypeUInt16: {
        auto values = array.template asValueArray<kvs::UInt16>();
        return cvt::detail::GetMinMaxValuesImpl( values, veclen, number_of_nodes );
    }
    case kvs::Type::TypeID::TypeUInt32: {
        auto values = array.template asValueArray<kvs::UInt32>();
        return cvt::detail::GetMinMaxValuesImpl( values, veclen, number_of_nodes );
    }
    case kvs::Type::TypeID::TypeUInt64: {
        auto values = array.template asValueArray<kvs::UInt64>();
        return cvt::detail::GetMinMaxValuesImpl( values, veclen, number_of_nodes );
    }
    case kvs::Type::TypeID::TypeReal64:
    default: {
        auto values = array.template asValueArray<kvs::Real64>();
        return cvt::detail::GetMinMaxValuesImpl( values, veclen, number_of_nodes );
    }
    }
}

inline void MinVec3( kvs::Vec3& v0, const kvs::Vec3& v1 )
{
    v0[0] = std::min( v0[0], v1[0] );
    v0[1] = std::min( v0[1], v1[1] );
    v0[2] = std::min( v0[2], v1[2] );
}

inline void MaxVec3( kvs::Vec3& v0, const kvs::Vec3& v1 )
{
    v0[0] = std::max( v0[0], v1[0] );
    v0[1] = std::max( v0[1], v1[1] );
    v0[2] = std::max( v0[2], v1[2] );
}
} // namespace detail
} // namespace cvt

namespace cvt
{
namespace detail
{
int GetKvsCellTypeId( const std::string& cell_type_expr );
} // namespace detail
} // namespace cvt

namespace cvt
{

/**
 * A PFI IO.
 *
 * Currently, only writing is supported.
 */
class UnstructuredPfi
{
public:
    /**
     * Construct a PFI IO.
     *
     * \param[in] number_of_components The number of node components, or 'veclen'.
     * \param[in] last_time_step The max time step. Zero-based indices.
     * \param[in] max_sub_volume_id The max sub volume ID. One-based indices.
     */
    UnstructuredPfi( int number_of_components, int last_time_step = 0, int max_sub_volume_id = 1 );

public:
    /**
     * Register a file for a PFI file.
     *
     * \param[in] object A KVS object.
     * \param[in] time_step A time step.
     * \param[in] sub_volume_id A sub volume ID.
     */
    void registerObject( kvs::KVSMLUnstructuredVolumeObject* object, int time_step = 0,
                         int sub_volume_id = 0 );
    /**
     * Register a file for a PFI file.
     *
     * \param[in] object A KVS object.
     * \param[in] time_step A time step.
     * \param[in] sub_volume_id A sub volume ID.
     */
    void registerObject( kvs::KVSMLStructuredVolumeObject* object, int time_step = 0,
                         int sub_volume_id = 0 );
    /**
     * Register a file for a PFI file.
     *
     * \param[in] output A data from a converter task.
     * \param[in] key A local file name prefix.
     */
    void registerObject( const ConverterTaskOutput* output, const std::string& key );
    /**
     * Write to a PFI file.
     *
     * The function will write to
     * -    '<directory>/<base>.pfi' on POSIX or
     * -    '<directory\\<base>.pfi' on Windows.
     *
     * \param[in] directory A directory name.
     * \param[in] base A file name base.
     * \return `true` on success, otherwise `false`.
     */
    template <typename PathLike0, typename PathLike1>
    bool write( PathLike0 directory, PathLike1 base )
    {
        try
        {
            cvt::filesystem::path path = directory;
            path.make_preferred();
            if ( !cvt::filesystem::exists( path ) )
            {
                if ( !cvt::filesystem::create_directories( path ) )
                {
                    kvsMessageError( "Failed to create the directory" );
                    return false;
                }
            }
            path /= ( std::string( base ) + ".pfi" );

            return write( path.string() );
        }
        catch ( std::exception& e )
        {
            throw e;
            return false;
        }
    }
    /**
     * Write to a PFI file.
     *
     * \param[in] filename A file name.
     * \return `true` on success, otherwise `false`.
     */
    bool write( const std::string& filename ) { return this->write( filename.c_str() ); }
    /**
     * Write to a PFI file.
     *
     * \param[in] filename A file name.
     * \return `true` on success, otherwise `false`.
     */
    bool write( const char* filename );
    /**
     * Print infomation to `Stream` .
     *
     * \tparam Stream The type of `stream` .
     * \param[inout] stream A stream.
     * \param[in] indent_count An indent count.
     */
    template <typename Stream>
    void print( Stream& stream, int indent_count = 0 )
    {
        std::string indent( indent_count, ' ' );
        std::cout << indent << "Type of element : " << type_of_cells << std::endl;
        std::cout << indent << "Last time step : " << last_time_step << std::endl;
        std::cout << indent << "Max sub volume ID : " << max_sub_volume_id << std::endl;
        std::cout << indent << "Number of components : " << number_of_component << std::endl;
        std::cout << std::string( indent_count + 4, ' ' )
                  << "Min. externals: " << min_external_coords << std::endl;
        std::cout << std::string( indent_count + 4, ' ' )
                  << "Max. externals: " << max_external_coords << std::endl;

        for ( int time_step = 0; time_step <= last_time_step; ++time_step )
        {
            std::cout << indent << "Time step : " << time_step << std::endl;

            for ( int s = 0; s < max_sub_volume_id; ++s )
            {
                std::cout << std::string( indent_count + 2, ' ' ) << "Sub volume : " << ( s + 1 )
                          << std::endl;
                std::cout << std::string( indent_count + 4, ' ' )
                          << "Node count : " << node_counts[time_step][s] << std::endl;
                std::cout << std::string( indent_count + 4, ' ' )
                          << "Element count : " << cell_counts[time_step][s] << std::endl;
                std::cout << std::string( indent_count + 4, ' ' )
                          << "Min. coords: " << min_object_coords[time_step][s] << std::endl;
                std::cout << std::string( indent_count + 4, ' ' )
                          << "Max. coords: " << max_object_coords[time_step][s] << std::endl;
            }

            for ( int i = 0; i < number_of_component; ++i )
            {
                std::cout << std::string( indent_count + 2, ' ' ) << "Min. Max. value (" << i
                          << ") : " << min_values[time_step][i] << " , " << max_values[time_step][i]
                          << std::endl;
            }
        }
    }

public:
    int number_of_component;
    int max_sub_volume_id;
    int last_time_step;
    std::vector<std::vector<int>> node_counts;
    std::vector<std::vector<int>> cell_counts;
    int type_of_cells;
    kvs::Vec3 min_external_coords;
    kvs::Vec3 max_external_coords;
    std::vector<std::vector<kvs::Vec3>> min_object_coords;
    std::vector<std::vector<kvs::Vec3>> max_object_coords;
    std::vector<std::vector<float>> min_values;
    std::vector<std::vector<float>> max_values;
};
} // namespace cvt

#endif // CVT__PFI_H_INCLUDE
