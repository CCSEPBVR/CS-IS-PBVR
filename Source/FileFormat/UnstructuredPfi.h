/*----------------------------------------------------------------------------
 *
 *  Copyright (c) Visualization Laboratory, Kyoto University.
 *  All rights reserved.
 *  See http://www.viz.media.kyoto-u.ac.jp/kvs/copyright/ for details.
 */
#ifndef CVT__UNSTRUCTURED_PFI_H_INCLUDE
#define CVT__UNSTRUCTURED_PFI_H_INCLUDE

#include <experimental/filesystem>
#include <string>
#include <vector>

#include "kvs/KVSMLUnstructuredVolumeObject"
#include "kvs/Vector3"

namespace cvt
{

/**
 * A PFI IO.
 */
class UnstructuredPfi
{
public:
    /**
     * Construct a PFI IO.
     *
     * \param [in] number_of_components The number of node components, or 'veclen'.
     * \param [in] last_time_step The max time step. Zero-based indices.
     * \param [in] max_sub_volume_id The max sub volume ID. One-based indices.
     */
    UnstructuredPfi( int number_of_components, int last_time_step = 0, int max_sub_volume_id = 1 );

public:
    /**
     * Register a file for a PFI file.
     *
     * \param [in] object A KVS object.
     * \param [in] time_step A time step.
     * \param [in] sub_volume_id A sub volume ID.
     */
    void registerObject( kvs::KVSMLUnstructuredVolumeObject* object, int time_step = 0,
                         int sub_volume_id = 0 );
    /**
     * Write to a PFI file.
     *
     * The function will write to
     * -    '<directory>/<base>.pfi' on POSIX or
     * -    '<directory\<base>.pfi' on Windows.
     *
     * \param [in] directory A directory name.
     * \param [in] base A file name base.
     * \return `true` on success, otherwise `false`.
     */
    template <typename PathLike0, typename PathLike1>
    bool write( PathLike0 directory, PathLike1 base )
    {
        try
        {
            std::experimental::filesystem::path path = directory;
            path.make_preferred();
            if ( !std::experimental::filesystem::exists( path ) )
            {
                std::experimental::filesystem::create_directories( path );
            }

            if ( !std::experimental::filesystem::exists( path ) )
            {
                throw std::runtime_error( "Failed to find the directory" );
            }
            path /= ( std::string( base ) + ".pfi" );

            return write( path.c_str() );
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
     * \param [in] filename A file name.
     * \return `true` on success, otherwise `false`.
     */
    bool write( const std::string& filename ) { return this->write( filename.c_str() ); }
    /**
     * Write to a PFI file.
     *
     * \param [in] filename A file name.
     * \return `true` on success, otherwise `false`.
     */
    bool write( const char* filename );

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
