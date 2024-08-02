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
#ifndef CVT__PFL_H_INCLUDE
#define CVT__PFL_H_INCLUDE

#include <forward_list>
#include <fstream>
#include <string>

#include "kvs/Message"

#include "Filesystem.h"

namespace cvt
{

/**
 * A PFL IO.
 *
 * Currently, only writing is supported.
 */
class Pfl
{
public:
    /**
     * Construct a PFL IO.
     */
    Pfl() {}

public:
    /**
     * Register a PFI file.
     *
     * \param[in] directory A directory name.
     * \param[in] base A file name base.
     */
    template <typename PathLike0, typename PathLike1>
    void registerPfi( PathLike0&& directory, PathLike1&& base )
    {
        std::filesystem::path path = directory;
        path.make_preferred();
        path /= ( std::string( base ) + ".pfi" );
        pfi_list.push_front( path.string() );
    }
    /**
     * Register a PFI file.
     *
     * \param[in] path A PFI file path.
     */
    template <typename PathLike>
    void registerPfi( PathLike&& path )
    {
        pfi_list.push_front( std::string( path ) );
    }
    /**
     * Write to a PFL file.
     *
     * The function will write to
     * -    '<directory>/<base>.pfl' on POSIX or
     * -    '<directory\<base>.pfl' on Windows.
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
            path /= ( std::string( base ) + ".pfl" );

            return write( path.string() );
        }
        catch ( std::exception& e )
        {
            kvsMessageError( e.what() );
            return false;
        }
    }
    /**
     * Write to a PFL file.
     *
     * \param[in] filename A file name.
     * \return `true` on success, otherwise `false`.
     */
    bool write( const std::string& filename ) { return this->write( filename.c_str() ); }
    /**
     * Write to a PFL file.
     *
     * \param[in] filename A file name.
     * \return `true` on success, otherwise `false`.
     */
    bool write( const char* filename )
    {
        std::ofstream stream( filename );

        stream << "#PBVR PFI FILES" << std::endl;

        for ( auto& p : pfi_list )
        {
            stream << p << std::endl;
        }

        return true;
    }

private:
    std::forward_list<std::string> pfi_list;
};
} // namespace cvt

#endif // CVT__PFL_H_INCLUDE