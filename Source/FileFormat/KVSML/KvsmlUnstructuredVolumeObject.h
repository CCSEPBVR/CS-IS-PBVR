/*----------------------------------------------------------------------------
 *
 *  Copyright (c) Visualization Laboratory, Kyoto University.
 *  All rights reserved.
 *  See http://www.viz.media.kyoto-u.ac.jp/kvs/copyright/ for details.
 */
#ifndef CVT__CVT_KVSML_UNSTRUCTURED_VOLUME_OBJECT_H_INCLUDE
#define CVT__CVT_KVSML_UNSTRUCTURED_VOLUME_OBJECT_H_INCLUDE

#include <iomanip>
#include <sstream>
#include <string>

#include "kvs/KVSMLUnstructuredVolumeObject"

#include "Filesystem.h"

namespace cvt
{

/**
 * A KVSML file IO for an unstructured volume object.
 */
class KvsmlUnstructuredVolumeObject : public kvs::KVSMLUnstructuredVolumeObject
{
public:
    /**
     * Construct a KVSML file IO.
     *
     * The instance from this constructor is for writing.
     */
    KvsmlUnstructuredVolumeObject(): kvs::KVSMLUnstructuredVolumeObject() {}
    /**
     * Construct a KVSML file IO and read an object from a file.
     *
     * \param [in] filename A file name.
     */
    KvsmlUnstructuredVolumeObject( const std::string& filename ):
        kvs::KVSMLUnstructuredVolumeObject( filename )
    {
    }

public:
    /**
     * Write a KVSML file for PBVR.
     *
     * This function will write to
     * -   '<directory>/<base>_<time_step>_<sub_volume_id>_<sub_volume_count>.kvsml' on POSIX or
     * -   '<directory>\<base>_<time_step>_<sub_volume_id>_<sub_volume_count>.kvsml' on Windows.
     *
     * \param [in] directory A directory name.
     * \param [in] base A file name base.
     * \param [in] time_step A time step.
     * \param [in] sub_volume_id A sub volume ID.
     * \param [in] sub_volume_count A total sub volume count.
     * \return `true` on success, otherwise `false`.
     */
    template <typename PathLike0, typename PathLike1>
    bool write( PathLike0 directory, PathLike1 base, int time_step = 0, int sub_volume_id = 1,
                int sub_volume_count = 1 )
    {
        try
        {
            std::stringstream stream;

            stream << base << "_" << std::setfill( '0' ) << std::right << std::setw( 5 )
                   << time_step << "_" << std::setfill( '0' ) << std::right << std::setw( 7 )
                   << sub_volume_id << "_" << std::setfill( '0' ) << std::right << std::setw( 7 )
                   << sub_volume_count << ".kvsml";

            std::filesystem::path path = directory;
            path.make_preferred();
            if ( !std::filesystem::exists( path ) )
            {
                std::filesystem::create_directories( path );
            }

            if ( !std::filesystem::exists( path ) )
            {
                throw std::runtime_error( "Failed to find the directory" );
            }
            path /= stream.str();

            return kvs::KVSMLUnstructuredVolumeObject::write( path.string() );
        }
        catch ( ... )
        {
            setSuccess( false );

            return false;
        }
    }
};
} // namespace cvt
#endif // CVT__CVT_KVSML_UNSTRUCTURED_VOLUME_OBJECT_H_INCLUDE