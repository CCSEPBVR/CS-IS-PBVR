/****************************************************************************/
/**
 *  @file File.cpp
 */
/*----------------------------------------------------------------------------
 *
 *  Copyright (c) Visualization Laboratory, Kyoto University.
 *  All rights reserved.
 *  See http://www.viz.media.kyoto-u.ac.jp/kvs/copyright/ for details.
 *
 *  $Id: File.cpp 633 2010-10-10 05:12:37Z naohisa.sakamoto $
 */
/****************************************************************************/
#include "File.h"
#include <kvs/Platform>
#if defined ( KVS_PLATFORM_WINDOWS )
#include <windows.h>
#else
#include <sys/stat.h>
#include <cstring>
#include <cerrno>
#endif
#include <iostream>
#include <fstream>
#include <kvs/Message>


namespace
{
#ifdef PATH_MAX
const size_t MaxPathLength = PATH_MAX;
#else
const size_t MaxPathLength = 4096;
#endif
}

namespace
{

/*==========================================================================*/
/**
 *  Get absolute path of given path.
 *  @param path [in] path
 *  @return absolute path
 */
/*==========================================================================*/
std::string GetAbsolutePath( const std::string& path )
{
    char absolute_path[::MaxPathLength];

#if defined ( KVS_PLATFORM_WINDOWS )
    _fullpath( absolute_path, const_cast<char*>( path.c_str() ), ::MaxPathLength );
#else
    if ( !realpath( path.c_str(), absolute_path ) )
    {
        kvsMessageError("%s", strerror( errno ) );
        return("");
    }
#endif

    return( absolute_path );
}

} // end of namespace


namespace kvs
{

/*==========================================================================*/
/**
 *  Constructor.
 */
/*==========================================================================*/
File::File( void )
    : m_file_path( "" )
    , m_path_name( "" )
    , m_file_name( "" )
    , m_base_name( "" )
    , m_extension( "" )
{
}

/*==========================================================================*/
/**
 *  Constructor.
 *  @param file_path [in] file path
 */
/*==========================================================================*/
File::File( const std::string& file_path )
    : m_file_path( "" )
    , m_path_name( "" )
    , m_file_name( "" )
    , m_base_name( "" )
    , m_extension( "" )
{
    this->parse( file_path );
}

/*==========================================================================*/
/**
 *  Constructor.
 *  @param file [in] file
 */
/*==========================================================================*/
File::File( const File& file )
{
    m_file_path = file.m_file_path;
    m_path_name = file.m_path_name;
    m_file_name = file.m_file_name;
    m_base_name = file.m_base_name;
    m_extension = file.m_extension;
}

/*==========================================================================*/
/**
 *  Destructor.
 */
/*==========================================================================*/
File::~File( void )
{
}

/*==========================================================================*/
/**
 *  Comparison operator.
 *  @param file [in] file
 */
/*==========================================================================*/
const bool File::operator <( const File& file ) const
{
    return( m_file_path < file.m_file_path );
}

/*==========================================================================*/
/**
 *  Comparison operator.
 *  @param file [in] file
 */
/*==========================================================================*/
const bool File::operator ==( const File& file ) const
{
    return( m_file_path == file.m_file_path );
}

/*==========================================================================*/
/**
 *  Get file path.
 *  @param absolute [in] flag for determing whether path is absolute-path
 *  @return file path
 */
/*==========================================================================*/
const std::string File::filePath( bool absolute ) const
{
    return( absolute ? ::GetAbsolutePath( m_file_path ) : m_file_path );
}

/*==========================================================================*/
/**
 *  Get path name.
 *  @param absolute [in] flag for determing whether path is absolute-path
 *  @return path name
 */
/*==========================================================================*/
const std::string File::pathName( bool absolute ) const
{
    return( absolute ? ::GetAbsolutePath( m_path_name ) : m_path_name );
}

/*==========================================================================*/
/**
 *  Get file name.
 *  @return file name
 */
/*==========================================================================*/
const std::string File::fileName( void ) const
{
    return( m_file_name );
}

/*==========================================================================*/
/**
 *  Get base name.
 *  @return base name
 */
/*==========================================================================*/
const std::string File::baseName( void ) const
{
    return( m_base_name );
}

/*==========================================================================*/
/**
 *  Get extension.
 *  @param complete [in] flag for determing whether extension is complete
 *  @return extension
 */
/*==========================================================================*/
const std::string File::extension( bool complete ) const
{
    if ( complete )
    {
        return( m_extension );
    }
    else
    {
        int last_dot_pos = m_extension.find_last_of( '.' );
        return( m_extension.substr( last_dot_pos + 1 ) );
    }
}

/*==========================================================================*/
/**
 *  Get file size.
 *  @return file size [byte]
 */
/*==========================================================================*/
const size_t File::byteSize( void ) const
{
    if ( this->isExisted() )
    {
        std::ifstream file( m_file_path.c_str(), std::ios::in | std::ios::binary );
        if ( file.fail() )
        {
            kvsMessageError( "Cannot open file: %s.", m_file_path.c_str() );
            return( 0 );
        }

        size_t byte_size = 0;
        std::streamoff p = file.tellg();
        {
            file.seekg( 0, std::ios::end );
            byte_size = static_cast<size_t>( file.tellg() );
        }
        file.seekg( p );

        file.close();

        return( byte_size );
    }

    return( 0 );
}

/*==========================================================================*/
/**
 *  Test to determine whether given file is a file.
 *  @return true, if given file is file
 */
/*==========================================================================*/
const bool File::isFile( void ) const
{
#if defined ( KVS_PLATFORM_WINDOWS )
    WIN32_FIND_DATAA find_data;
    if ( FindFirstFileA( m_file_path.c_str(), &find_data ) == INVALID_HANDLE_VALUE ) { return( false ); };
    return( ( find_data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY ) != FILE_ATTRIBUTE_DIRECTORY );
#else
    struct stat filestat;
    if ( stat( m_file_path.c_str(), &filestat ) ) { return( false ); }
    return( filestat.st_mode & S_IFREG );
#endif
}

/*==========================================================================*/
/**
 *  Test to determin whether given file is existed.
 *  @return true, if given file is existed
 */
/*==========================================================================*/
const bool File::isExisted( void ) const
{
    return( this->isFile() );
}

/*==========================================================================*/
/**
 *  Parse given file path.
 *  @param file_path [in] file path
 *  @return true, if the parse process is done successfully
 */
/*==========================================================================*/
const bool File::parse( const std::string& file_path )
{
    m_file_path = file_path;

    int last_sep_pos = file_path.find_last_of( File::Separator() );
    if ( last_sep_pos < 0 )
    {
        m_path_name = ".";
        m_file_name = file_path;
    }
    else if ( last_sep_pos == 0 )
    {
        m_path_name = File::Separator();
        m_file_name = file_path.substr( last_sep_pos + 1 );
    }
    else
    {
        m_path_name = std::string( file_path.begin(), file_path.begin() + last_sep_pos );
        m_file_name = file_path.substr( last_sep_pos + 1 );
    }

    int first_dot_pos = m_file_name.find_first_of( '.' );
    if ( first_dot_pos < 0 )
    {
        m_base_name = m_file_name;
        m_extension = "";
    }
    else
    {
        m_base_name = std::string( m_file_name.begin(),
                                   m_file_name.begin() + first_dot_pos );
        m_extension = std::string( m_file_name.begin() + first_dot_pos + 1,
                                   m_file_name.end() );
    }

    return( true );
}

/*==========================================================================*/
/**
 *  Get file separator.
 *  @return file separator
 */
/*==========================================================================*/
const std::string File::Separator( void )
{
#if defined ( KVS_PLATFORM_WINDOWS )
    return( "\\" );
#else
    return( "/" );
#endif
}

} // end of namespace kvs
