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
 *  $Id: Argument.cpp 625 2010-10-01 07:39:27Z naohisa.sakamoto $
 */
/****************************************************************************/
#include "Argument.h"
#include "CommandName.h"


namespace kvscheck
{

/*===========================================================================*/
/**
 *  @brief  Constructs a new Argument class.
 *  @param  argc [in] argument count
 *  @param  argv [in] argument values
 */
/*===========================================================================*/
Argument::Argument( int argc, char** argv ):
    kvs::CommandLine( argc, argv, kvscheck::CommandName )
{
    add_help_option();
    add_option("version", "Output KVS version. (optional)");
    add_option("platform", "Output platforma information. (optional)");
    add_option("compiler", "Output compiler information. (optional)");
    add_option("sizeof", "Output 'sizeof' information. (optional)");
    add_option("support", "Output supported library information. (optional)");
    add_option("minmax", "Output min/max information. (optional)");
    add_option("opengl", "Output OpenGL information. (optional)");
    add_option("extension", "Output OpenGL extension information. (optional)");
    add_option("file", "Output file information. (optional)");
    add_value("input value", false);
}

} // end of namespace kvscheck
