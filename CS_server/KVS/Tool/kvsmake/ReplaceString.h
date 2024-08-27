/****************************************************************************/
/**
 *  @file ReplaceString.h
 */
/*----------------------------------------------------------------------------
 *
 *  Copyright (c) Visualization Laboratory, Kyoto University.
 *  All rights reserved.
 *  See http://www.viz.media.kyoto-u.ac.jp/kvs/copyright/ for details.
 *
 *  $Id: ReplaceString.h 626 2010-10-01 07:45:20Z naohisa.sakamoto $
 */
/****************************************************************************/
#ifndef KVSMAKE__REPLACE_STRING_H_INCLUDE
#define KVSMAKE__REPLACE_STRING_H_INCLUDE

#include <string>


namespace kvsmake
{

const std::string ReplaceString(
    const std::string& source,
    const std::string& pattern,
    const std::string& placement );


/*==========================================================================*/
/**
 *  Replaces the 'pattern' to the 'placement' in the 'source'.
 *
 *  @param source    [in] .
 *  @param pattern   [in] .
 *  @param placement [in] .
 */
/*==========================================================================*/
inline const std::string ReplaceString(
    const std::string& source,
    const std::string& pattern,
    const std::string& placement )
{
    std::string result( source );

    for ( std::string::size_type pos = 0 ;
          std::string::npos != ( pos = result.find( pattern, pos ) );
          pos += placement.size() )
    {
        result.replace( pos, pattern.size(), placement );
    }

    return( result );
}

} // end of namespace kvsmake

#endif // KVSMAKE__REPLACE_STRING_H_INCLUDE
