/*****************************************************************************/
/**
 *  @file   Argument.cpp
 */
/*----------------------------------------------------------------------------
 *
 *  Copyright (c) Visualization Laboratory, Kyoto University.
 *  All rights reserved.
 *  See http://www.viz.media.kyoto-u.ac.jp/kvs/copyright/ for details.
 *
 *  $Id: Argument.cpp 602 2010-08-19 02:43:34Z naohisa.sakamoto $
 */
/*****************************************************************************/
#include "Argument.h"
#include "CommandName.h"
#include "fld2kvsml.h"
#include "ucd2kvsml.h"
#include "img2img.h"
#include "tet2tet.h"


namespace kvsconv
{

/*===========================================================================*/
/**
 *  @brief  Constructs a new argument class.
 *  @param  argc [in] argument count
 *  @param  argv [in] argument values
 */
/*===========================================================================*/
Argument::Argument( int argc, char** argv ):
    kvs::CommandLine( argc, argv, kvsconv::CommandName )
{
    add_help_option();

    // Help option for each converter.
    add_option("help", "Output help message for each converter. (ex. -help fld2kvsml)", 1 );

    // Converter commands.
    add_option( kvsconv::fld2kvsml::CommandName, kvsconv::fld2kvsml::Description, 0 );
    add_option( kvsconv::ucd2kvsml::CommandName, kvsconv::ucd2kvsml::Description, 0 );
    add_option( kvsconv::img2img::CommandName, kvsconv::img2img::Description, 0 );
    add_option( kvsconv::tet2tet::CommandName, kvsconv::tet2tet::Description, 0 );

    // Input value.
    add_value("input value");
}

/*===========================================================================*/
/**
 *  @brief  Constructs a new common argument class.
 *  @param  argc [in] argument count
 *  @param  argv [in] argument values
 */
/*===========================================================================*/
Argument::Common::Common( int argc, char** argv ):
    kvs::CommandLine( argc, argv, kvsconv::CommandName )
{
    this->set_options();
}

/*===========================================================================*/
/**
 *  @brief  Constructs a new Argument::Common class.
 *  @param  argc [in] argument count
 *  @param  argv [in] argument values
 *  @param  converter [in] converter name
 */
/*===========================================================================*/
Argument::Common::Common( int argc, char** argv, const std::string& converter ):
    kvs::CommandLine( argc, argv, kvsconv::CommandName + " -" + converter )
{
    this->set_options();
}

/*===========================================================================*/
/**
 *  @brief  Set options for the common argument.
 */
/*===========================================================================*/
void Argument::Common::set_options( void )
{
    add_help_option("help");

    add_option("output", "Output filename. (default: <input_basename>.<output_extension>)", 1, false );

    add_value("input data file");
}

} // end of namespace kvsconv
