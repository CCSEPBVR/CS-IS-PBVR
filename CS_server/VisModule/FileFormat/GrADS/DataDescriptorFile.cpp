/*****************************************************************************/
/**
 *  @file   DataDescriptorFile.cpp
 *  @author Naohisa Sakamoto
 */
/*----------------------------------------------------------------------------
 *
 *  Copyright (c) Visualization Laboratory, Kyoto University.
 *  All rights reserved.
 *  See http://www.viz.media.kyoto-u.ac.jp/kvs/copyright/ for details.
 *
 *  $Id: DataDescriptorFile.cpp 999 2011-11-30 07:08:26Z naohisa.sakamoto@gmail.com $
 */
/*****************************************************************************/
#include "DataDescriptorFile.h"
#include <string>
#include <sstream>
#include <iostream>
#include <vismodule/Message>
#include <vismodule/Tokenizer>


namespace
{

bool GetLine( std::istream & in, std::string & str )
{
    if ( !in ) return( false );

    char ch;
    str = "";

    // Windows:CRLF(\r\n), Unix:LF(\n), Mac:CR(\r)
    while ( in.get( ch ) )
    {
        if ( ch == '\0' ) { break; }
        if ( ch == '\n' ) { break; }
        if ( ch == '\r' ) { ch = in.peek(); if ( ch == '\n' ) in.get( ch ); break; }
        str += ch;
    }

    return( true );
}

}


namespace vismodule
{

namespace grads
{

/*===========================================================================*/
/**
 *  @brief  Constructs a new DataDescriptorFile class.
 */
/*===========================================================================*/
DataDescriptorFile::DataDescriptorFile( void )
{
}

/*===========================================================================*/
/**
 *  @brief  Returns DSET entry information.
 *  @return DSET entry information
 */
/*===========================================================================*/
const vismodule::grads::DSet& DataDescriptorFile::dset( void ) const
{
    return( m_dset );
}

/*===========================================================================*/
/**
 *  @brief  Returns UNDEF entry information.
 *  @return UNDEF entry information
 */
/*===========================================================================*/
const vismodule::grads::Undef& DataDescriptorFile::undef( void ) const
{
    return( m_undef );
}

/*===========================================================================*/
/**
 *  @brief  Returns TITLE entry information.
 *  @return TITLE entry information
 */
/*===========================================================================*/
const vismodule::grads::Title& DataDescriptorFile::title( void ) const
{
    return( m_title );
}

/*===========================================================================*/
/**
 *  @brief  Returns OPTIONS entry information.
 *  @return OPTIONS entry information
 */
/*===========================================================================*/
const vismodule::grads::Options& DataDescriptorFile::options( void ) const
{
    return( m_options );
}

/*===========================================================================*/
/**
 *  @brief  Returns XDEF entry information.
 *  @return XDEF entry information
 */
/*===========================================================================*/
const vismodule::grads::XDef& DataDescriptorFile::xdef( void ) const
{
    return( m_xdef );
}

/*===========================================================================*/
/**
 *  @brief  Returns YDEF entry information.
 *  @return YDEF entry information
 */
/*===========================================================================*/
const vismodule::grads::YDef& DataDescriptorFile::ydef( void ) const
{
    return( m_ydef );
}

/*===========================================================================*/
/**
 *  @brief  Returns ZDEF entry information.
 *  @return ZDEF entry information
 */
/*===========================================================================*/
const vismodule::grads::ZDef& DataDescriptorFile::zdef( void ) const
{
    return( m_zdef );
}

/*===========================================================================*/
/**
 *  @brief  Returns TDEF entry information.
 *  @return TDEF entry information
 */
/*===========================================================================*/
const vismodule::grads::TDef& DataDescriptorFile::tdef( void ) const
{
    return( m_tdef );
}

/*===========================================================================*/
/**
 *  @brief  Returns VARS entry information.
 *  @return VARS entry information
 */
/*===========================================================================*/
const vismodule::grads::Vars& DataDescriptorFile::vars( void ) const
{
    return( m_vars );
}

/*===========================================================================*/
/**
 *  @brief  Read data descriptor file.
 *  @param  ifs [in] input file stream
 *  @return true, if the reading process is done successfully
 */
/*===========================================================================*/
const bool DataDescriptorFile::read( std::ifstream& ifs )
{
    std::string line;
//    while ( std::getline( ifs, line ) )
    while ( ::GetLine( ifs, line ) )
    {
        // Trim white space.
        std::string blank(" ");
        line.erase( 0, line.find_first_not_of( blank ) );

        // Comment.
        if ( line[0] == '@' ) continue;
        if ( line[0] == '*' ) continue;

        // Check entry.
        vismodule::Tokenizer t( line, " \t\n" );
        std::string entry = t.token();

        if ( entry == "DSET" || entry == "dset" )
        {
            if ( !m_dset.read( line, ifs ) ) { visModuleMessageError("Cannot read DSET."); return( false ); }
        }

        if ( entry == "UNDEF" || entry == "undef" )
        {
            if ( !m_undef.read( line, ifs ) ) { visModuleMessageError("Cannot read UNDEF."); return( false ); }
        }

        if ( entry == "TITLE" || entry == "title" )
        {
            if ( !m_title.read( line, ifs ) ) { visModuleMessageError("Cannot read TITLE."); return( false ); }
        }

        if ( entry == "OPTIONS" || entry == "options" )
        {
            if ( !m_options.read( line, ifs ) ) { visModuleMessageError("Cannot read OPTIONS."); return( false ); }
        }

        if ( entry == "XDEF" || entry == "xdef" )
        {
            if ( !m_xdef.read( line, ifs ) ) { visModuleMessageError("Cannot read XDEF."); return( false ); }
        }

        if ( entry == "YDEF" || entry == "ydef" )
        {
            if ( !m_ydef.read( line, ifs ) ) { visModuleMessageError("Cannot read YDEF."); return( false ); }
        }

        if ( entry == "ZDEF" || entry == "zdef" )
        {
            if ( !m_zdef.read( line, ifs ) ) { visModuleMessageError("Cannot read ZDEF."); return( false ); }
        }

        if ( entry == "TDEF" || entry == "tdef" )
        {
            if ( !m_tdef.read( line, ifs ) ) { visModuleMessageError("Cannot read TDEF."); return( false ); }
        }

        if ( entry == "VARS" || entry == "vars" )
        {
            if ( !m_vars.read( line, ifs ) ) { visModuleMessageError("Cannot read VARS."); return( false ); }
        }
    }

    return( true );
}

} // end of namespace grads

} // end of namespace vismodule
