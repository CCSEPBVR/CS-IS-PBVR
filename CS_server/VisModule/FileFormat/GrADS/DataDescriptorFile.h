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
#ifndef VIS_MODULE__GRADS__DATA_DESCRIPTOR_H_INCLUDE
#define VIS_MODULE__GRADS__DATA_DESCRIPTOR_H_INCLUDE

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


namespace vismodule
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

    vismodule::grads::DSet m_dset; ///< data filename (DSET)
    vismodule::grads::Undef m_undef; ///< undefined data (UNDEF)
    vismodule::grads::Title m_title; ///< brief description of the data (TITLE)
    vismodule::grads::Options m_options; ///< option list (OPTIONS)
    vismodule::grads::XDef m_xdef; ///< grid point values for the X dimension (XDEF)
    vismodule::grads::YDef m_ydef; ///< grid point values for the Y dimension (YDEF)
    vismodule::grads::ZDef m_zdef; ///< grid point values for the Z dimension (ZDEF)
    vismodule::grads::TDef m_tdef; ///< grid point values for the T dimension (TDEF)
    vismodule::grads::Vars m_vars; ///< all variables in the data set (VARS)

public:

    DataDescriptorFile( void );

public:

    const vismodule::grads::DSet& dset( void ) const;

    const vismodule::grads::Undef& undef( void ) const;

    const vismodule::grads::Title& title( void ) const;

    const vismodule::grads::Options& options( void ) const;

    const vismodule::grads::XDef& xdef( void ) const;

    const vismodule::grads::YDef& ydef( void ) const;

    const vismodule::grads::ZDef& zdef( void ) const;

    const vismodule::grads::TDef& tdef( void ) const;

    const vismodule::grads::Vars& vars( void ) const;

    const bool read( std::ifstream& ifs );
};

} // end of namespace grads

} // end of namespace vismodule

#endif // VIS_MODULE__GRADS__DATA_DESCRIPTOR_H_INCLUDE
