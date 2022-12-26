/*----------------------------------------------------------------------------
 *
 *  Copyright (c) Visualization Laboratory, Kyoto University.
 *  All rights reserved.
 *  See http://www.viz.media.kyoto-u.ac.jp/kvs/copyright/ for details.
 */
#ifndef CVT__UNSTRUCTURED_PFI_H_INCLUDE
#define CVT__UNSTRUCTURED_PFI_H_INCLUDE

#include <string>
#include <vector>

#include "kvs/KVSMLStructuredVolumeObject"
#include "kvs/KVSMLUnstructuredVolumeObject"
#include "kvs/Message"
#include "kvs/Vector3"

#include "Filesystem.h"

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
            std::filesystem::path path = directory;
            path.make_preferred();
            if ( !std::filesystem::exists( path ) )
            {
                if ( !std::filesystem::create_directories( path ) )
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
        std::cout << indent << "Type of element : " << type_of_elements << std::endl;
        std::cout << indent << "Last time step : " << last_time_step << std::endl;
        std::cout << indent << "Max sub volume ID : " << max_sub_volume_id << std::endl;
        std::cout << indent << "Number of components : " << number_of_component << std::endl;

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
                          << "Element count : " << element_counts[time_step][s] << std::endl;
                std::cout << std::string( indent_count + 4, ' ' )
                          << "Min. coords: " << min_object_coords[time_step][s] << std::endl;
                std::cout << std::string( indent_count + 4, ' ' )
                          << "Max. coords: " << max_object_coords[time_step][s] << std::endl;
                std::cout << std::string( indent_count + 4, ' ' )
                          << "Min. externals: " << min_external_coords[time_step][s] << std::endl;
                std::cout << std::string( indent_count + 4, ' ' )
                          << "Max. externals: " << max_external_coords[time_step][s] << std::endl;
            }

            for ( int i = 0; i < number_of_component; ++i )
            {
                std::cout << std::string( indent_count + 2, ' ' ) << "Min. Max. value (" << i
                          << ") : " << min_values[time_step][i] << " , " << max_values[time_step][i]
                          << std::endl;
            }
        }
    }

private:
    int number_of_component;
    int max_sub_volume_id;
    int last_time_step;
    std::vector<std::vector<int>> node_counts;
    std::vector<std::vector<int>> element_counts;
    int type_of_elements;
    std::vector<std::vector<kvs::Vec3>> min_external_coords;
    std::vector<std::vector<kvs::Vec3>> max_external_coords;
    std::vector<std::vector<kvs::Vec3>> min_object_coords;
    std::vector<std::vector<kvs::Vec3>> max_object_coords;
    std::vector<std::vector<float>> min_values;
    std::vector<std::vector<float>> max_values;
};
} // namespace cvt

#endif // CVT__PFI_H_INCLUDE
