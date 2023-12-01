/*****************************************************************************/
/**
 *  @file   Title.cpp
 *  @author Naohisa Sakamoto
 */
/*----------------------------------------------------------------------------
 *
 *  Copyright (c) Visualization Laboratory, Kyoto University.
 *  All rights reserved.
 *  See http://www.viz.media.kyoto-u.ac.jp/kvs/copyright/ for details.
 *
 *  $Id: Title.cpp 863 2011-07-20 10:40:29Z naohisa.sakamoto $
 */
/*****************************************************************************/
#include "Title.h"
#include <kvs/Tokenizer>


namespace kvs
{

namespace grads
{

/*===========================================================================*/
/**
 *  @brief  Reads TITLE entry.
 *  @param  line [in] read line
 *  @param  ifs [in] input file stream
 *  @return true, if the reading process is done successfully
 */
/*===========================================================================*/
const bool Title::read( std::string line, std::ifstream& ifs )
{
    kvs::Tokenizer t( line, " \t\n" );
    t.token(); // TITLE

    this->name = t.token();

    return( true );
}

} // end of namespace grads

} // end of namespace kvs
