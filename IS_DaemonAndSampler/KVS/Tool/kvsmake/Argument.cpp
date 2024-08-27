/****************************************************************************/
/**
 *  @file Argument.cpp
 */
/*----------------------------------------------------------------------------
 *
 *  Copyright (c) Visualization Laboratory, Kyoto University.
 *  All rights reserved.
 *  See http://www.viz.media.kyoto-u.ac.jp/kvs/copyright/ for details.
 *
 *  $Id: Argument.cpp 626 2010-10-01 07:45:20Z naohisa.sakamoto $
 */
/****************************************************************************/
#include "Argument.h"
#include "CommandName.h"
#include <kvs/Compiler>


namespace kvsmake
{

/*===========================================================================*/
/**
 *  @brief  Constructs a new Argument class.
 *  @param  argc [in] argument count
 *  @param  argv [in] argument values
 */
/*===========================================================================*/
Argument::Argument( int argc, char** argv ):
    kvs::CommandLine( argc, argv, kvsmake::CommandName )
{
    add_help_option();

    add_value( "project_name/make_options", false );

    add_option( "g",      "Generate a makefile.", 1 );
    add_option( "G",      "Generate a makefile and set it's target name to the current directory name", 0 );

#if defined ( KVS_COMPILER_VC )
    add_option( "v",      "generate a project file for VC.", 1 );
    add_option( "vc",     "generate a project file for VC.", 1 );
    add_option( "vcproj", "generate a project file for VC.", 1 );

    add_option( "cuda", "generate a project file for VC and CUDA.", 1 );
#endif

    add_option( "q",      "Generate a project file for Qt.", 1 );
    add_option( "qt",     "Generate a project file for Qt.", 1 );
    add_option( "qtproj", "Generate a project file for Qt.", 1 );
}

} // end of namespace kvsmake
