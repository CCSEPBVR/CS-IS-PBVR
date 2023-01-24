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
#ifndef CVT__CVT_KVSML_UNSTRUCTURED_VOLUME_OBJECT_H_INCLUDE
#define CVT__CVT_KVSML_UNSTRUCTURED_VOLUME_OBJECT_H_INCLUDE

#include <fstream>
#include <iomanip>
#include <regex>
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
     * \param[in] filename A file name.
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
     * -   '<directory>\\<base>_<time_step>_<sub_volume_id>_<sub_volume_count>.kvsml' on Windows.
     *
     * \param[in] directory A directory name.
     * \param[in] base A file name base.
     * \param[in] time_step A time step. Zero-based numbering.
     * \param[in] sub_volume_id A sub volume ID. One-based numbering.
     * \param[in] sub_volume_count A total sub volume count.
     * \param[in] share_0_step_coords Reuse the first step coords and connection file if this set
     * \return `true` on success, otherwise `false`.
     */
    template <typename PathLike0, typename PathLike1>
    bool write( PathLike0 directory, PathLike1 base, int time_step = 0, int sub_volume_id = 1,
                int sub_volume_count = 1, bool share_0_step_coords = false )
    {
        setSuccess( false );

        try
        {
            std::stringstream stream;

            stream << base << "_" << std::setfill( '0' ) << std::right << std::setw( 5 )
                   << time_step << "_" << std::setfill( '0' ) << std::right << std::setw( 7 )
                   << sub_volume_id << "_" << std::setfill( '0' ) << std::right << std::setw( 7 )
                   << sub_volume_count;
            auto merged_base = stream.str();

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
            path /= merged_base;
            path += ".kvsml";

            if ( !kvs::KVSMLUnstructuredVolumeObject::write( path.string() ) )
            {
                return false;
            }

            // Replace coords and connection file
            if ( share_0_step_coords && time_step != 0 )
            {
                std::filesystem::path tmp = directory;
                tmp /= merged_base;
                tmp += ".tmp";
                std::filesystem::rename( path, tmp );
                std::stringstream sss;
                int zero_time_step = 0;
                sss << base << "_" << std::setfill( '0' ) << std::right << std::setw( 5 )
                    << zero_time_step << "_" << std::setfill( '0' ) << std::right << std::setw( 7 )
                    << sub_volume_id << "_" << std::setfill( '0' ) << std::right << std::setw( 7 )
                    << sub_volume_count;
                auto zero_base = sss.str();

                {
                    std::ifstream src( tmp.string() );
                    std::ofstream dst( path.string(), std::ios::trunc );

                    std::string line;
                    while ( std::getline( src, line ) )
                    {
                        auto line0 =
                            std::regex_replace( line, std::regex( merged_base + "_coord.dat" ),
                                                zero_base + "_coord.dat" );
                        auto line1 =
                            std::regex_replace( line0, std::regex( merged_base + "_connect.dat" ),
                                                zero_base + "_connect.dat" );
                        dst << line1 << std::endl;
                    }
                }

                std::filesystem::remove( tmp );

                std::filesystem::path coord = directory;
                coord /= merged_base;
                coord += "_coord.dat";
                std::filesystem::remove( coord );

                std::filesystem::path connect = directory;
                connect /= merged_base;
                connect += "_connect.dat";
                std::filesystem::remove( connect );
            }

            setSuccess( true );
            return true;
        }
        catch ( ... )
        {
            return false;
        }
    }
};
} // namespace cvt
#endif // CVT__CVT_KVSML_UNSTRUCTURED_VOLUME_OBJECT_H_INCLUDE
