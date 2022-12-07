/****************************************************************************/
/**
 *  @file WriteVCProject.cpp
 */
/*----------------------------------------------------------------------------
 *
 *  Copyright (c) Visualization Laboratory, Kyoto University.
 *  All rights reserved.
 *  See http://www.viz.media.kyoto-u.ac.jp/kvs/copyright/ for details.
 *
 *  $Id: WriteVCProject.cpp 674 2011-04-20 04:18:33Z naohisa.sakamoto $
 */
/****************************************************************************/
#include "WriteVCProject.h"

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

void WriteVCProject( const std::string& project_name )
{
    //  Open a template file.
    std::ifstream in( kvsmake::VCProjectTemplate.c_str() );
    if ( !in.is_open() )
    {
        kvsMessageError( "Cannot open %s.", kvsmake::VCProjectTemplate.c_str() );
        return;
    }

    //  Open a project file.
    const std::string filename( project_name + ".vcproj" );

    std::ofstream out( filename.c_str() );
    if ( !out.is_open() )
    {
        kvsMessageError( "Cannot open %s.", filename.c_str() );
        return;
    }

    kvsmake::WriteVCProjectBody( in, out, project_name );
}

void WriteVCProjectBody(
    std::ifstream&     in,
    std::ofstream&     out,
    const std::string& project_name )
{
    //  Search the project's condition.
    std::string vc_version( "" );
    std::string headers( "" );
    std::string sources( "" );

    vc_version = KVS_COMPILER_VERSION;

    // Search cpp files and h files.
    const kvs::Directory current_dir( "." );
    const kvs::FileList& file_list = current_dir.fileList();

    kvs::FileList::const_iterator       iter = file_list.begin();
    const kvs::FileList::const_iterator end  = file_list.end();

    while ( iter != end )
    {
        if ( iter->extension() == "h" )
        {
            headers += ( "      <File RelativePath=\".\\" + iter->fileName() + "\"/>\n" );
        }
        else if ( iter->extension() == "cpp" )
        {
            sources += ( "      <File RelativePath=\".\\" + iter->fileName() + "\"/>\n" );
        }

        ++iter;
    }

    // Additional dependencies (static libraries).
    std::string libraries("");
#if defined( KVS_SUPPORT_GLEW )
    libraries.append("kvsSupportGLEW.lib glew32.lib ");
#endif
#if defined( KVS_SUPPORT_GLUT )
    libraries.append("kvsSupportGLUT.lib glut32.lib");
#endif
    libraries.append("kvsCore.lib ");
    libraries.append("glu32.lib ");
    libraries.append("opengl32.lib");

    // Preprocessor definitions.
    std::string definitions("");
#if defined( KVS_SUPPORT_GLEW )
    definitions.append("KVS_SUPPORT_GLEW;");
#endif
#if defined( KVS_SUPPORT_GLUT )
    definitions.append("KVS_SUPPORT_GLUT;");
#endif
    definitions.append("WIN32;");
    definitions.append("_MBCS;");
    definitions.append("NOMINMAX;");
    definitions.append("_SCL_SECURE_NO_DEPRECATE;");
    definitions.append("_CRT_SECURE_NO_DEPRECATE;");
    definitions.append("_CRT_NONSTDC_NO_DEPRECATE;");

    std::string definitions_debug( definitions );
    definitions_debug.append("_DEBUG;");
    definitions_debug.append("KVS_ENABLE_DEBUG;");

    std::string definitions_release( definitions );
    definitions_release.append("NDEBUG;");

    // Write a project file.
    while ( !in.eof() )
    {
        std::string line( "" );

        std::getline( in, line );

        line = kvsmake::ReplaceString( line, "VC_VERSION_REPLACED_BY_KVSMAKE", vc_version );
        line = kvsmake::ReplaceString( line, "PROJECT_NAME_REPLACED_BY_KVSMAKE", project_name );
        line = kvsmake::ReplaceString( line, "HEADERS_REPLACED_BY_KVSMAKE", headers );
        line = kvsmake::ReplaceString( line, "SOURCES_REPLACED_BY_KVSMAKE", sources );
        line = kvsmake::ReplaceString( line, "DEFINITIONS_DEBUG_REPLACED_BY_KVSMAKE", definitions_debug );
        line = kvsmake::ReplaceString( line, "DEFINITIONS_RELEASE_REPLACED_BY_KVSMAKE", definitions_release );
        line = kvsmake::ReplaceString( line, "LIBRARIES_REPLACED_BY_KVSMAKE", libraries );

        out << line << std::endl;
    }
}

} // end of namespace kvsmake
