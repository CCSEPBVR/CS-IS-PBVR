/*****************************************************************************/
/**
 *  @file   DSet.h
 *  @author Naohisa Sakamoto
 */
/*----------------------------------------------------------------------------
 *
 *  Copyright (c) Visualization Laboratory, Kyoto University.
 *  All rights reserved.
 *  See http://www.viz.media.kyoto-u.ac.jp/kvs/copyright/ for details.
 *
 *  $Id: DSet.h 863 2011-07-20 10:40:29Z naohisa.sakamoto $
 */
/*****************************************************************************/
#ifndef KVS__GRADS__DSET_H_INCLUDE
#define KVS__GRADS__DSET_H_INCLUDE

#include <string>
#include <fstream>


namespace kvs
{

namespace grads
{

/*===========================================================================*/
/**
 *  @brief  DSET entry.
 */
/*===========================================================================*/
struct DSet
{
    std::string name; ///< data filename

    const bool read( std::string line, std::ifstream& ifs );
};

} // end of namespace grads

} // end of namespace kvs

#endif // KVS__GRADS__DSET_H_INCLUDE
