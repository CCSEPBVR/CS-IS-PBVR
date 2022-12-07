/****************************************************************************/
/**
 *  @file CompilerChecker.cpp
 */
/*----------------------------------------------------------------------------
 *
 *  Copyright (c) Visualization Laboratory, Kyoto University.
 *  All rights reserved.
 *  See http://www.viz.media.kyoto-u.ac.jp/kvs/copyright/ for details.
 *
 *  $Id: CompilerChecker.cpp 602 2010-08-19 02:43:34Z naohisa.sakamoto $
 */
/****************************************************************************/
#include "CompilerChecker.h"
#include <kvs/Compiler>


namespace kvscheck
{

/*===========================================================================*/
/**
 *  @brief  Constructs a new CompilerChecker class.
 */
/*===========================================================================*/
CompilerChecker::CompilerChecker( void )
{
    m_name    = kvs::Compiler::Name();
    m_version = kvs::Compiler::Version();
}

/*===========================================================================*/
/**
 *  @brief  Returns a compiler name.
 *  @return compiler name
 */
/*===========================================================================*/
const std::string& CompilerChecker::name( void ) const
{
    return( m_name );
}

/*===========================================================================*/
/**
 *  @brief  Returns a compiler version.
 *  @return compiler version
 */
/*===========================================================================*/
const std::string& CompilerChecker::version( void ) const
{
    return( m_version );
}

/*==========================================================================*/
/**
 *  @brief Outputs version information.
 *  @param os [in] output stream
 *  @param checker [in] compiler checker
 */
/*==========================================================================*/
std::ostream& operator << ( std::ostream& os, const CompilerChecker& checker )
{
    os << "Compiler: " << checker.name() << " (" << checker.version() << ")" << std::endl;

    return( os );
}

} // end of namespace kvscheck
