/****************************************************************************/
/**
 *  @file WriteMakefile.cpp
 */
/*----------------------------------------------------------------------------
 *
 *  Copyright (c) Visualization Laboratory, Kyoto University.
 *  All rights reserved.
 *  See http://www.viz.media.kyoto-u.ac.jp/kvs/copyright/ for details.
 *
 *  $Id: WriteMakefile.cpp 602 2010-08-19 02:43:34Z naohisa.sakamoto $
 */
/****************************************************************************/
#include "WriteMakefile.h"

#include <string>  // For std::string.
#include <fstream> // For std::ifstream, std::ofstream.

#include <kvs/Message>
#include <kvs/Compiler>

#if defined ( KVS_COMPILER_VC )
#include <kvs/Directory>
#include <kvs/FileList>
#include <kvs/File>
#endif

#include "Constant.h"
#include "ReplaceString.h"


namespace kvsmake
{

void WriteMakefile( const std::string& project_name )
{
    //  Open a template file.
    std::ifstream in( kvsmake::MakefileTemplate.c_str() );
    if ( !in.is_open() )
    {
        kvsMessageError( "Cannot open %s.", kvsmake::MakefileTemplate.c_str() );
        return;
    }

    //  Open a Makefile.
    std::ofstream out( kvsmake::Makefile.c_str() );
    if ( !out.is_open() )
    {
        kvsMessageError( "Cannot open %s.", kvsmake::Makefile.c_str() );
        return;
    }

    kvsmake::WriteMakefileBody( in, out, project_name );
}

void WriteMakefileBody(
    std::ifstream&     in,
    std::ofstream&     out,
    const std::string& project_name )
{
#if defined ( KVS_COMPILER_VC )
    // Search cpp files.
    std::string sources( "" );

    const kvs::Directory current_dir( "." );
    const kvs::FileList& file_list = current_dir.fileList();

    kvs::FileList::const_iterator       iter = file_list.begin();
    const kvs::FileList::const_iterator end  = file_list.end();

    while ( iter != end )
    {
        if ( iter->extension() == "cpp" )
        {
            sources += ( iter->fileName() + " \\\n" );
        }

        ++iter;
    }
#endif

    // Write a Makefile.
    while ( !in.eof() )
    {
        std::string line( "" );

        std::getline( in, line );

        line = kvsmake::ReplaceString( line, "PROJECT_NAME_REPLACED_BY_KVSMAKE", project_name );

#if defined ( KVS_COMPILER_VC )
        line = kvsmake::ReplaceString( line, "SOURCES_REPLACED_BY_KVSMAKE", sources );
#endif

        out << line << std::endl;
    }
}

} // end of namespace kvsmake
