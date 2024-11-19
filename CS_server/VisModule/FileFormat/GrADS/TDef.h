/*****************************************************************************/
/**
 *  @file   TDef.h
 *  @author Naohisa Sakamoto
 */
/*----------------------------------------------------------------------------
 *
 *  Copyright (c) Visualization Laboratory, Kyoto University.
 *  All rights reserved.
 *  See http://www.viz.media.kyoto-u.ac.jp/kvs/copyright/ for details.
 *
 *  $Id: TDef.h 863 2011-07-20 10:40:29Z naohisa.sakamoto $
 */
/*****************************************************************************/
#ifndef VIS_MODULE__GRADS__TDEF_H_INCLUDE
#define VIS_MODULE__GRADS__TDEF_H_INCLUDE

#include <string>
#include <fstream>


namespace vismodule
{

namespace grads
{

/*===========================================================================*/
/**
 *  @brief  TDEF entry.
 */
/*===========================================================================*/
struct TDef
{
    struct Start
    {
        int hour; ///< hour
        int minute; ///< minute
        int day; ///< day
        int month; ///< month
        int year; ///< year
    };

    struct Increment
    {
        int value; ///< value
        int unit; ///< unit (0:mn, 1:hr, 2:dy, 3:mo, 4:yr)
    };

    size_t num; ///< number of the time steps
    Start start; ///< start time/date
    Increment increment; ///< increment time

    const bool read( std::string line, std::ifstream& ifs );

    TDef& operator ++ ( void );
};

} // end of namespace grads

} // end of namespace vismodule

#endif // VIS_MODULE__GRADS__TDEF_H_INCLUDE
