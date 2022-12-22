/*----------------------------------------------------------------------------
 *
 *  Copyright (c) Visualization Laboratory, Kyoto University.
 *  All rights reserved.
 *  See http://www.viz.media.kyoto-u.ac.jp/kvs/copyright/ for details.
 */
#ifndef CVT__PFL_H_INCLUDE
#define CVT__PFL_H_INCLUDE

#include <forward_list>
#include <fstream>
#include <string>

#include "Filesystem.h"

namespace cvt
{

/**
 * A PFL IO.
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
     * \param [in] directory A directory name.
     * \param [in] base A file name base.
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
     * \param [in] path A PFI file path.
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
                std::filesystem::create_directories( path );
            }

            if ( !std::filesystem::exists( path ) )
            {
                throw std::runtime_error( "Failed to find the directory" );
            }
            path /= ( std::string( base ) + ".pfl" );

            return write( path.c_str() );
        }
        catch ( std::exception& e )
        {
            throw e;
            return false;
        }
    }
    /**
     * Write to a PFL file.
     *
     * \param [in] filename A file name.
     * \return `true` on success, otherwise `false`.
     */
    bool write( const std::string& filename ) { return this->write( filename.c_str() ); }
    /**
     * Write to a PFL file.
     *
     * \param [in] filename A file name.
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