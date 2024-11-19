/*****************************************************************************/
/**
 *  @file   GrADS.cpp
 *  @author Naohisa Sakamoto
 *  @brief  
 */
/*----------------------------------------------------------------------------
 *
 *  Copyright (c) Visualization Laboratory, Kyoto University.
 *  All rights reserved.
 *  See http://www.viz.media.kyoto-u.ac.jp/kvs/copyright/ for details.
 *
 *  $Id: GrADS.cpp 1000 2011-12-09 09:18:50Z naohisa.sakamoto $
 */
/*****************************************************************************/
#include "GrADS.h"
#include <cstdlib>
#include <cstdio>
#include <kvs/IgnoreUnusedVariable>
#include <kvs/File>
#include <kvs/Directory>
#include <kvs/String>


namespace
{

const char* MonthName[] = {
    "jan", "feb", "mar", "apr", "may", "jun", "jul", "aug", "sep", "oct", "nov", "dec"
};

const int DaysInMonth[] = {
    31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31
};

std::string ReplaceYear( const std::string& filename, const int year )
{
    if ( filename.find("%y2",0) != std::string::npos )
    {
        int y = year % 100;
        kvs::String replace( y );
        kvs::String file( filename );
        file.replace( "%y2", replace.toStdString() );
        return( file.toStdString() );
    }

    if ( filename.find("%y4",0) != std::string::npos )
    {
        int y = year;
        if ( y < 100 ) y = ( 50 <= y ) ? year + 1900 : year + 2000;
        kvs::String replace( y );
        kvs::String file( filename );
        file.replace( "%y4", replace.toStdString() );
        return( file.toStdString() );
    }

    return( filename );
}

std::string ReplaceMonth( const std::string& filename, const int month )
{
    if ( filename.find("%m1",0) != std::string::npos )
    {
        kvs::String replace( month );
        kvs::String file( filename );
        file.replace( "%m1", replace.toStdString() );
        return( file.toStdString() );
    }

    if ( filename.find("%m2",0) != std::string::npos )
    {
        char replace[3]; sprintf( replace, "%02d", month );
        kvs::String file( filename );
        file.replace( "%m2", std::string( replace ) );
        return( file.toStdString() );
    }

    if ( filename.find("%mc",0) != std::string::npos )
    {
        kvs::String file( filename );
        file.replace( "%mc", MonthName[month - 1] );
        return( file.toStdString() );
    }

    return( filename );
}

std::string ReplaceDay( const std::string& filename, const int day )
{
    if ( filename.find("%d1",0) != std::string::npos )
    {
        kvs::String replace( day );
        kvs::String file( filename );
        file.replace( "%d1", replace.toStdString() );
        return( file.toStdString() );
    }

    if ( filename.find("%d2",0) != std::string::npos )
    {
        char replace[3]; sprintf( replace, "%02d", day );
        kvs::String file( filename );
        file.replace( "%d2", std::string( replace ) );
        return( file.toStdString() );
    }

    return( filename );
}

std::string ReplaceHour( const std::string& filename, const int hour )
{
    if ( filename.find("%h1",0) != std::string::npos )
    {
        kvs::String replace( hour );
        kvs::String file( filename );
        file.replace( "%h1", replace.toStdString() );
        return( file.toStdString() );
    }

    if ( filename.find("%h2",0) != std::string::npos )
    {
        char replace[3]; sprintf( replace, "%02d", hour );
        kvs::String file( filename );
        file.replace( "%h2", std::string( replace ) );
        return( file.toStdString() );
    }

    if ( filename.find("%h3",0) != std::string::npos )
    {
        char replace[4]; sprintf( replace, "%03d", hour );
        kvs::String file( filename );
        file.replace( "%h3", std::string( replace ) );
        return( file.toStdString() );
    }

    return( filename );
}

std::string ReplaceMinute( const std::string& filename, const int minute )
{
    if ( filename.find("%n2",0) != std::string::npos )
    {
        char replace[3]; sprintf( replace, "%02d", minute );
        kvs::String file( filename );
        file.replace( "%n2", std::string( replace ) );
        return( file.toStdString() );
    }

    return( filename );
}

}


namespace kvs
{

/*===========================================================================*/
/**
 *  @brief  Constructs a new GrADS class.
 */
/*===========================================================================*/
GrADS::GrADS( void )
{
}

/*===========================================================================*/
/**
 *  @brief  Constructs a new GrADS class.
 *  @param  filename [in] filename
 */
/*===========================================================================*/
GrADS::GrADS( const std::string& filename )
{
    if( this->read( filename ) ) { m_is_success = true; }
    else { m_is_success = false; }
}

/*===========================================================================*/
/**
 *  @brief  Destroys the GrADS class.
 */
/*===========================================================================*/
GrADS::~GrADS( void )
{
}

/*===========================================================================*/
/**
 *  @brief  Returns data descriptor file.
 *  @return data descriptor file
 */
/*===========================================================================*/
const GrADS::DataDescriptorFile& GrADS::dataDescriptor( void ) const
{
    return( m_data_descriptor );
}

/*===========================================================================*/
/**
 *  @brief  Returns gridded binary data list.
 *  @return gridded binary data list
 */
/*===========================================================================*/
const GrADS::GriddedBinaryDataFileList& GrADS::dataList( void ) const
{
    return( m_data_list );
}

/*===========================================================================*/
/**
 *  @brief  Returns gridded binary data specified by the index.
 *  @return gridded binary data
 */
/*===========================================================================*/
const GrADS::GriddedBinaryDataFile& GrADS::data( const size_t index ) const
{
    return( m_data_list[index] );
}

/*===========================================================================*/
/**
 *  @brief  Reads GrADS data.
 *  @param  filename [in] filename
 *  @return true, if the reading process is done successfully
 */
/*===========================================================================*/
const bool GrADS::read( const std::string& filename )
{
    BaseClass::m_filename = filename;

    // Open file.
    std::ifstream ifs( m_filename.c_str(), std::ios::binary | std::ios::in );
    if( !ifs.is_open() )
    {
        kvsMessageError( "Cannot open %s.", m_filename.c_str() );
        ifs.close();
        BaseClass::m_is_success = false;
        return( BaseClass::m_is_success );
    }

    // Read control file.
    if ( !m_data_descriptor.read( ifs ) )
    {
        kvsMessageError( "Cannot read control file." );
        ifs.close();
        BaseClass::m_is_success = false;
        return( BaseClass::m_is_success );
    }

    const bool sequential = m_data_descriptor.options().find( kvs::grads::Options::Sequential );
    const bool big_endian = m_data_descriptor.options().find( kvs::grads::Options::BigEndian );

    // Read binary file. (The data is not loaded into the memory)
    std::string data_pathname;
    std::string template_data_filename = m_data_descriptor.dset().name;
    const kvs::File template_data_file( template_data_filename );
    if ( template_data_file.pathName() == template_data_file.pathName( true ) ) // absolute path
    {
        data_pathname = template_data_file.pathName();
    }
    else
    {
        data_pathname = kvs::File( filename ).pathName();
        if ( template_data_filename[0] == '^' ) { template_data_filename.erase( 0, 1 ); }
    }

    size_t counter = 0;
    kvs::Directory directory( data_pathname );
    kvs::grads::TDef tdef = m_data_descriptor.tdef();
    kvs::FileList::const_iterator file = directory.fileList().begin();
    kvs::FileList::const_iterator last = directory.fileList().end();
    while ( file != last )
    {
        std::string data_filename = template_data_filename;
        data_filename = ::ReplaceYear( data_filename, tdef.start.year );
        data_filename = ::ReplaceMonth( data_filename, tdef.start.month );
        data_filename = ::ReplaceDay( data_filename, tdef.start.day );
        data_filename = ::ReplaceHour( data_filename, tdef.start.hour );
        data_filename = ::ReplaceMinute( data_filename, tdef.start.minute );

        if ( file->fileName() == data_filename )
        {
            const std::string sep = kvs::File::Separator();
            const std::string path = directory.directoryPath( true );
            GriddedBinaryDataFile data;
            data.setFilename( path + sep + data_filename );
            data.setSequential( sequential );
            data.setBigEndian( big_endian );
            m_data_list.push_back( data );
            ++counter;
            ++tdef;
        }
        if ( counter == m_data_descriptor.tdef().num ) break;

        ++file;
    }

    return( true );
}

/*===========================================================================*/
/**
 *  @brief  Writes GrADS data.
 *  @param  filename [in] filename
 *  @return false (not yet implemented)
 */
/*===========================================================================*/
const bool GrADS::write( const std::string& filename )
{
    kvs::IgnoreUnusedVariable( filename );

    return( false );
}

} // end of namespace kvs
