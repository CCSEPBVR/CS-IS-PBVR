/****************************************************************************/
/**
 *  @file PlatformChecker.h
 */
/*----------------------------------------------------------------------------
 *
 *  Copyright (c) Visualization Laboratory, Kyoto University.
 *  All rights reserved.
 *  See http://www.viz.media.kyoto-u.ac.jp/kvs/copyright/ for details.
 *
 *  $Id: PlatformChecker.h 602 2010-08-19 02:43:34Z naohisa.sakamoto $
 */
/****************************************************************************/
#ifndef KVSCHECK__PLATFORM_CHECKER_H_INCLUDE
#define KVSCHECK__PLATFORM_CHECKER_H_INCLUDE

#include <string>
#include <iostream>


namespace kvscheck
{

/*==========================================================================*/
/**
 *  Platform checker class.
 */
/*==========================================================================*/
class PlatformChecker
{
protected:

    std::string m_name; ///< platform name
    std::string m_cpu;  ///< CPU name

public:

    PlatformChecker( void );

public:

    const std::string& name( void ) const;

    const std::string& cpu( void ) const;

    friend std::ostream& operator << ( std::ostream& os, const PlatformChecker& checker );
};

} // end of namespace kvscheck

#endif // KVSCHECK__PLATFORM_CHECKER_H_INCLUDE
