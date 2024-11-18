/*****************************************************************************/
/**
 *  @file   GridPoint.h
 *  @author Naohisa Sakamoto
 */
/*----------------------------------------------------------------------------
 *
 *  Copyright (c) Visualization Laboratory, Kyoto University.
 *  All rights reserved.
 *  See http://www.viz.media.kyoto-u.ac.jp/kvs/copyright/ for details.
 *
 *  $Id: XYZDef.h 863 2011-07-20 10:40:29Z naohisa.sakamoto $
 */
/*****************************************************************************/
#ifndef KVS__GRADS__XYZDEF_H_INCLUDE
#define KVS__GRADS__XYZDEF_H_INCLUDE

#include <string>
#include <fstream>
#include <kvs/ValueArray>


namespace kvs
{

namespace grads
{

/*===========================================================================*/
/**
 *  @brief  XDEF/YDEF/ZDEF entry.
 */
/*===========================================================================*/
struct XYZDef
{
    enum MappingMethod
    {
        Linear,
        Levels,
        GausT62,
        GausR15,
        GausR20,
        GausR30,
        GausR40
    };

    size_t num; ///< number of grid points
    MappingMethod mapping; ///< mapping method
    kvs::ValueArray<kvs::Real32> values; ///< values

    const bool read( std::string line, std::ifstream& ifs );
};

} // end of namespace grads

} // end of namespace kvs

#endif // KVS__GRADS__XYZDEF_H_INCLUDE
