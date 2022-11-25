/****************************************************************************/
/**
 *  @file main.cpp
 */
/*----------------------------------------------------------------------------
 *
 *  Copyright (c) Visualization Laboratory, Kyoto University.
 *  All rights reserved.
 *  See http://www.viz.media.kyoto-u.ac.jp/kvs/copyright/ for details.
 *
 *  $Id: main.cpp 660 2011-01-15 03:58:22Z naohisa.sakamoto $
 */
/****************************************************************************/
#include <cstdlib>
#include <string>
#include <kvs/MemoryDebugger>
#include <kvs/Compiler>
#include <kvs/Message>
#include <kvs/Directory>
#include <kvs/File>
#include "Argument.h"
#include "Constant.h"
#include "WriteMakefile.h"
#if defined ( KVS_COMPILER_VC )
#include "WriteVCProject.h"
#include "WriteVCProjectCUDA.h"
#include "WriteVCXProject.h"
#endif
#include "WriteQtProject.h"

KVS_MEMORY_DEBUGGER;

/*==========================================================================*/
/**
 *  @brief  Main function for kvsmake.
 *  @param  argc [in] argument count
 *  @param  argv [in] argument values
 */
/*==========================================================================*/
int main( int argc, char** argv )
{
    KVS_MEMORY_DEBUGGER__SET_ARGUMENT( argc, argv );

    kvsmake::Argument argument( argc, argv );
    if ( !argument.parse() )
    {
        exit( EXIT_FAILURE );
    }

    if ( argument.hasOption( "g" ) )
    {
        const std::string project_name( argument.optionValue<std::string>( "g" ) );

        kvsmake::WriteMakefile( project_name );

        return( 0 );
    }
    if ( argument.hasOption( "G" ) )
    {
        const kvs::Directory dir( "." );
        const std::string project_name( dir.directoryName() );

        kvsmake::WriteMakefile( project_name );

        return( 0 );
    }

    if ( argument.hasOption( "q" ) )
    {
        const std::string project_name( argument.optionValue<std::string>( "q" ) );

        kvsmake::WriteQtProject( project_name );

        return( 0 );
    }
    if ( argument.hasOption( "qt" ) )
    {
        const std::string project_name( argument.optionValue<std::string>( "qt" ) );

        kvsmake::WriteQtProject( project_name );

        return( 0 );
    }
    if ( argument.hasOption( "qtproj" ) )
    {
        const std::string project_name( argument.optionValue<std::string>( "qtproj" ) );

        kvsmake::WriteQtProject( project_name );

        return( 0 );
    }

#if defined ( KVS_COMPILER_VC )
    if ( argument.hasOption( "v" ) )
    {
        const std::string project_name( argument.optionValue<std::string>( "v" ) );

#if KVS_COMPILER_VERSION_GREATER_OR_EQUAL( 10, 0 )
        kvsmake::WriteVCXProject( project_name );
#else
        kvsmake::WriteVCProject( project_name );
#endif
        return( 0 );
    }
    if ( argument.hasOption( "vc" ) )
    {
        const std::string project_name( argument.optionValue<std::string>( "vc" ) );

#if KVS_COMPILER_VERSION_GREATER_OR_EQUAL( 10, 0 )
        kvsmake::WriteVCXProject( project_name );
#else
        kvsmake::WriteVCProject( project_name );
#endif

        return( 0 );
    }
    if ( argument.hasOption( "vcproj" ) )
    {
        const std::string project_name( argument.optionValue<std::string>( "vcproj" ) );

#if KVS_COMPILER_VERSION_GREATER_OR_EQUAL( 10, 0 )
        kvsmake::WriteVCXProject( project_name );
#else
        kvsmake::WriteVCProject( project_name );
#endif

        return( 0 );
    }
    if ( argument.hasOption( "cuda" ) )
    {
        const std::string project_name( argument.optionValue<std::string>( "cuda" ) );

        kvsmake::WriteVCProjectCUDA( project_name );

        return( 0 );
    }
#endif

    if ( !kvs::File( kvsmake::Makefile ).isExisted() )
    {
        kvsMessageError( "Cannot find %s.", kvsmake::Makefile.c_str() );
        exit( EXIT_FAILURE );
    }

    std::string make_argument;
    for ( int i = 1; i < argc; ++i )
    {
        make_argument += std::string( " " ) + std::string( argv[i] );
    }

    const std::string command =
        kvsmake::MakeCommand + std::string( " -f " ) + kvsmake::Makefile + make_argument;

    return( system( command.c_str() ) );
}
