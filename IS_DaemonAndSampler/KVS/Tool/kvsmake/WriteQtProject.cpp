/****************************************************************************/
/**
 *  @file WriteQtProject.cpp
 */
/*----------------------------------------------------------------------------
 *
 *  Copyright (c) Visualization Laboratory, Kyoto University.
 *  All rights reserved.
 *  See http://www.viz.media.kyoto-u.ac.jp/kvs/copyright/ for details.
 *
 *  $Id: WriteQtProject.cpp 993 2011-10-15 00:26:38Z naohisa.sakamoto@gmail.com $
 */
/****************************************************************************/
#include "WriteQtProject.h"

#include <string>  // For std::string.
#include <fstream> // For std::ifstream, std::ofstream.

#include <kvs/Message>
#include <kvs/Directory>
#include <kvs/FileList>
#include <kvs/File>

#include "Constant.h"
#include "ReplaceString.h"


namespace kvsmake
{

void WriteQtProject( const std::string& project_name )
{
    //  Open a template file.
    std::ifstream in( kvsmake::QtProjectTemplate.c_str() );
    if ( !in.is_open() )
    {
        kvsMessageError( "Cannot open %s.", kvsmake::QtProjectTemplate.c_str() );
        return;
    }

    //  Open a project file.
    const std::string filename( project_name + ".pro" );

    std::ofstream out( filename.c_str() );
    if ( !out.is_open() )
    {
        kvsMessageError( "Cannot open %s.", filename.c_str() );
        return;
    }

    kvsmake::WriteQtProjectBody( in, out, project_name );
}

void WriteQtProjectBody(
    std::ifstream&     in,
    std::ofstream&     out,
    const std::string& project_name )
{
    //  Search the project's condition.
    std::string headers( "" );
    std::string sources( "" );
    std::string qresrcs( "" );

    // Search cpp files and h files.
    const kvs::Directory current_dir( "." );
    const kvs::FileList& file_list = current_dir.fileList();

    kvs::FileList::const_iterator       iter = file_list.begin();
    const kvs::FileList::const_iterator end  = file_list.end();

    while ( iter != end )
    {
        if ( iter->extension() == "h" )
        {
            headers += ( iter->fileName() + " \\\n" );
        }
        else if ( iter->extension() == "cpp" )
        {
            sources += ( iter->fileName() + " \\\n" );
        }
        else if ( iter->extension() == "qrc" )
        {
            qresrcs += ( iter->fileName() + " \\\n" );
        }

        ++iter;
    }

    // Write a project file.
    while ( !in.eof() )
    {
        std::string line( "" );

        std::getline( in, line );

        line = kvsmake::ReplaceString( line, "PROJECT_NAME_REPLACED_BY_KVSMAKE", project_name );
        line = kvsmake::ReplaceString( line, "HEADERS_REPLACED_BY_KVSMAKE", headers );
        line = kvsmake::ReplaceString( line, "SOURCES_REPLACED_BY_KVSMAKE", sources );
        line = kvsmake::ReplaceString( line, "QRESRCS_REPLACED_BY_KVSMAKE", qresrcs );

        out << line << std::endl;
    }
}

} // end of namespace kvsmake
