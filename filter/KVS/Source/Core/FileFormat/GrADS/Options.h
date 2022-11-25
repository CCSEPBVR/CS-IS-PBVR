/*****************************************************************************/
/**
 *  @file   Options.h
 *  @author Naohisa Sakamoto
 */
/*----------------------------------------------------------------------------
 *
 *  Copyright (c) Visualization Laboratory, Kyoto University.
 *  All rights reserved.
 *  See http://www.viz.media.kyoto-u.ac.jp/kvs/copyright/ for details.
 *
 *  $Id: Options.h 863 2011-07-20 10:40:29Z naohisa.sakamoto $
 */
/*****************************************************************************/
#ifndef KVS__GRADS__OPTIONS_H_INCLUDE
#define KVS__GRADS__OPTIONS_H_INCLUDE

#include <list>
#include <string>
#include <fstream>


namespace kvs
{

namespace grads
{

/*===========================================================================*/
/**
 *  @brief  OPTIONS entry.
 */
/*===========================================================================*/
struct Options
{
    enum Keyword
    {
        Pascals,
        Yrev,
        Zrev,
        Template,
        Sequential,
        DayCalendar365,
        ByteSwapped,
        BigEndian,
        LittleEndian,
        Cray32bitIEEE
    };

    std::list<Keyword> values; ///< keyword list

    const bool read( std::string line, std::ifstream& ifs );

    const bool find( const Keyword keyword ) const;
};

} // end of namespace grads

} // end of namespace kvs

#endif // KVS__GRADS__OPTIONS_H_INCLUDE
