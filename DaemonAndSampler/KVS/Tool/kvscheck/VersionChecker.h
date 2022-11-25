/****************************************************************************/
/**
 *  @file VersionChecker.h
 */
/*----------------------------------------------------------------------------
 *
 *  Copyright (c) Visualization Laboratory, Kyoto University.
 *  All rights reserved.
 *  See http://www.viz.media.kyoto-u.ac.jp/kvs/copyright/ for details.
 *
 *  $Id: VersionChecker.h 602 2010-08-19 02:43:34Z naohisa.sakamoto $
 */
/****************************************************************************/
#ifndef KVSCHECK__VERSION_CHECKER_H_INCLUDE
#define KVSCHECK__VERSION_CHECKER_H_INCLUDE

#include <string>
#include <iostream>


namespace kvscheck
{

/*==========================================================================*/
/**
 *  KVS version checker class.
 */
/*==========================================================================*/
class VersionChecker
{
protected:

    std::string m_name; ///< KVS version name

public:

    VersionChecker( void );

public:

    const std::string& name( void ) const;

    friend std::ostream& operator << ( std::ostream& os, const VersionChecker& checker );
};

} // end of namespace kvscheck

#endif // KVSCHECK__VERSION_CHECKER_H_INCLUDE
