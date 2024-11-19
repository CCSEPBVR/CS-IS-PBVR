/*****************************************************************************/
/**
 *  @file   Title.h
 *  @author Naohisa Sakamoto
 */
/*----------------------------------------------------------------------------
 *
 *  Copyright (c) Visualization Laboratory, Kyoto University.
 *  All rights reserved.
 *  See http://www.viz.media.kyoto-u.ac.jp/kvs/copyright/ for details.
 *
 *  $Id: Title.h 863 2011-07-20 10:40:29Z naohisa.sakamoto $
 */
/*****************************************************************************/
#ifndef VIS_MODULE__GRADS__TITLE_H_INCLUDE
#define VIS_MODULE__GRADS__TITLE_H_INCLUDE

#include <string>
#include <fstream>


namespace vismodule
{

namespace grads
{

/*===========================================================================*/
/**
 *  @brief  TITLE entry.
 */
/*===========================================================================*/
struct Title
{
    std::string name; ///< name

    const bool read( std::string line, std::ifstream& ifs );
};

} // end of namespace grads

} // end of namespace vismodule

#endif // VIS_MODULE__GRADS__TITLE_H_INCLUDE
