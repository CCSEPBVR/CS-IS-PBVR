/*****************************************************************************/
/**
 *  @file   Vars.h
 *  @author Naohisa Sakamoto
 */
/*----------------------------------------------------------------------------
 *
 *  Copyright (c) Visualization Laboratory, Kyoto University.
 *  All rights reserved.
 *  See http://www.viz.media.kyoto-u.ac.jp/kvs/copyright/ for details.
 *
 *  $Id: Vars.h 863 2011-07-20 10:40:29Z naohisa.sakamoto $
 */
/*****************************************************************************/
#ifndef VIS_MODULE__GRADS__VARS_H_INCLUDE
#define VIS_MODULE__GRADS__VARS_H_INCLUDE

#include <list>
#include <string>
#include <fstream>


namespace vismodule
{

namespace grads
{

/*===========================================================================*/
/**
 *  @brief  VARS entry.
 */
/*===========================================================================*/
struct Vars
{
    struct Var
    {
        std::string varname; ///< variable name
        int levs; ///< levels
        float units; ///< unit
        std::string description; ///< description
    };

    std::list<Var> values; ///< value list

    const bool read( std::string line, std::ifstream& ifs );
};

} // end of namespace grads

} // end of namespace vismodule

#endif // VIS_MODULE__GRADS__VARS_H_INCLUDE
