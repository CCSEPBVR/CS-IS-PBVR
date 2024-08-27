/*****************************************************************************/
/**
 *  @file   Undef.cpp
 *  @author Naohisa Sakamoto
 */
/*----------------------------------------------------------------------------
 *
 *  Copyright (c) Visualization Laboratory, Kyoto University.
 *  All rights reserved.
 *  See http://www.viz.media.kyoto-u.ac.jp/kvs/copyright/ for details.
 *
 *  $Id: Undef.cpp 869 2011-07-25 05:11:49Z naohisa.sakamoto $
 */
/*****************************************************************************/
#include "Undef.h"
#include <kvs/Tokenizer>
#include <cstdlib>


namespace kvs
{

namespace grads
{

/*===========================================================================*/
/**
 *  @brief  Reads UNDEF entry.
 *  @param  line [in] read line
 *  @param  ifs [in] input file stream
 *  @return true, if the reading process is done successfully
 */
/*===========================================================================*/
const bool Undef::read( std::string line, std::ifstream& ifs )
{
    kvs::Tokenizer t( line, " \t\n" );
    t.token(); // UNDEF

    this->value = atof( t.token().c_str() );

    return( true );
}

} // end of namespace grads

} // end of namespace kvs
