/*****************************************************************************/
/**
 *  @file   DataDescriptorFile.h
 *  @author Naohisa Sakamoto
 */
/*----------------------------------------------------------------------------
 *
 *  Copyright (c) Visualization Laboratory, Kyoto University.
 *  All rights reserved.
 *  See http://www.viz.media.kyoto-u.ac.jp/kvs/copyright/ for details.
 *
 *  $Id: DataDescriptorFile.h 863 2011-07-20 10:40:29Z naohisa.sakamoto $
 */
/*****************************************************************************/
#ifndef KVS__GRADS__DATA_DESCRIPTOR_H_INCLUDE
#define KVS__GRADS__DATA_DESCRIPTOR_H_INCLUDE

#include <fstream>
#include "DSet.h"
#include "Undef.h"
#include "Title.h"
#include "Options.h"
#include "XDef.h"
#include "YDef.h"
#include "ZDef.h"
#include "TDef.h"
#include "Vars.h"


namespace kvs
{

namespace grads
{

/*===========================================================================*/
/**
 *  @brief  DataDescriptorFile class.
 */
/*===========================================================================*/
class DataDescriptorFile
{
private:

    kvs::grads::DSet m_dset; ///< data filename (DSET)
    kvs::grads::Undef m_undef; ///< undefined data (UNDEF)
    kvs::grads::Title m_title; ///< brief description of the data (TITLE)
    kvs::grads::Options m_options; ///< option list (OPTIONS)
    kvs::grads::XDef m_xdef; ///< grid point values for the X dimension (XDEF)
    kvs::grads::YDef m_ydef; ///< grid point values for the Y dimension (YDEF)
    kvs::grads::ZDef m_zdef; ///< grid point values for the Z dimension (ZDEF)
    kvs::grads::TDef m_tdef; ///< grid point values for the T dimension (TDEF)
    kvs::grads::Vars m_vars; ///< all variables in the data set (VARS)

public:

    DataDescriptorFile( void );

public:

    const kvs::grads::DSet& dset( void ) const;

    const kvs::grads::Undef& undef( void ) const;

    const kvs::grads::Title& title( void ) const;

    const kvs::grads::Options& options( void ) const;

    const kvs::grads::XDef& xdef( void ) const;

    const kvs::grads::YDef& ydef( void ) const;

    const kvs::grads::ZDef& zdef( void ) const;

    const kvs::grads::TDef& tdef( void ) const;

    const kvs::grads::Vars& vars( void ) const;

    const bool read( std::ifstream& ifs );
};

} // end of namespace grads

} // end of namespace kvs

#endif // KVS__GRADS__DATA_DESCRIPTOR_H_INCLUDE
