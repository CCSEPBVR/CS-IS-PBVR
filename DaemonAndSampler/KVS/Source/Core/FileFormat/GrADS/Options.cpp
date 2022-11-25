/*****************************************************************************/
/**
 *  @file   Options.cpp
 *  @author Naohisa Sakamoto
 */
/*----------------------------------------------------------------------------
 *
 *  Copyright (c) Visualization Laboratory, Kyoto University.
 *  All rights reserved.
 *  See http://www.viz.media.kyoto-u.ac.jp/kvs/copyright/ for details.
 *
 *  $Id: Options.cpp 868 2011-07-25 02:21:51Z naohisa.sakamoto $
 */
/*****************************************************************************/
#include "Options.h"
#include <kvs/Tokenizer>
#include <algorithm>


namespace kvs
{

namespace grads
{

/*===========================================================================*/
/**
 *  @brief  Reads OPTIONS entry.
 *  @param  line [in] read line
 *  @param  ifs [in] input file stream
 *  @return true, if the reading process is done successfully
 */
/*===========================================================================*/
const bool Options::read( std::string line, std::ifstream& ifs )
{
    kvs::Tokenizer t( line, " \t\n" );
    t.token(); // OPTIONS

    this->values.clear();
    while ( !t.isLast() )
    {
        const std::string value = t.token();
        if ( value == "pascals" ) this->values.push_back( Pascals );
        else if ( value == "yrev" ) this->values.push_back( Yrev );
        else if ( value == "zrev" ) this->values.push_back( Zrev );
        else if ( value == "template" ) this->values.push_back( Template );
        else if ( value == "sequential" ) this->values.push_back( Sequential );
        else if ( value == "365_day_calendar" ) this->values.push_back( DayCalendar365 );
        else if ( value == "byteswapped" ) this->values.push_back( ByteSwapped );
        else if ( value == "big_endian" ) this->values.push_back( BigEndian );
        else if ( value == "little_endian" ) this->values.push_back( LittleEndian );
        else if ( value == "cray_32bit_ieee" ) this->values.push_back( Cray32bitIEEE );
    }

    return( true );
}

/*===========================================================================*/
/**
 *  @brief  Finds keyword.
 *  @param  keyword [in] keyword
 *  @return true, if the keyword is found
 */
/*===========================================================================*/
const bool Options::find( const Options::Keyword keyword ) const
{
    std::list<Keyword>::const_iterator begin = this->values.begin();
    std::list<Keyword>::const_iterator end = this->values.end();

    return( std::find( begin, end, keyword ) != end );
}

} // end of namespace grads

} // end of namespace kvs
