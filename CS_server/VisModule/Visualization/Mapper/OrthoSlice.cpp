/****************************************************************************/
/**
 *  @file OrthoSlice.cpp
 */
/*----------------------------------------------------------------------------
 *
 *  Copyright (c) Visualization Laboratory, Kyoto University.
 *  All rights reserved.
 *  See http://www.viz.media.kyoto-u.ac.jp/kvs/copyright/ for details.
 *
 *  $Id: OrthoSlice.cpp 602 2010-08-19 02:43:34Z naohisa.sakamoto $
 */
/****************************************************************************/
#include "OrthoSlice.h"
#include <vismodule/Matrix33>


namespace
{

const vismodule::Matrix33f Normal(
    1.0f, 0.0f, 0.0f,
    0.0f, 1.0f, 0.0f,
    0.0f, 0.0f, 1.0f );

}

namespace vismodule
{

/*==========================================================================*/
/**
 *  Construct a new OrthoSlice class.
 */
/*==========================================================================*/
OrthoSlice::OrthoSlice( void ):
    m_aligned_axis( OrthoSlice::XAxis )
{
}

/*==========================================================================*/
/**
 *  @brief  Construct and create a slice plane as a polygon object.
 *  @param  volume [in] pointer to the volume object
 *  @param  position [in] position on the specified axis
 *  @param  axis [in] aligned axis
 *  @param  transfer_function [in] transfer function
 */
/*==========================================================================*/
OrthoSlice::OrthoSlice(
    const vismodule::VolumeObjectBase* volume,
    const float                  position,
    const AlignedAxis            axis,
    const vismodule::TransferFunction& transfer_function ):
    vismodule::SlicePlane(
        volume,
        ::Normal[axis] * position,
        ::Normal[axis],
        transfer_function )
{
}

/*===========================================================================*/
/**
 *  @brief  Sets a plane information.
 *  @param  position [in] slice position
 *  @param  axis [in] slice orientation
 */
/*===========================================================================*/
void OrthoSlice::setPlane( const float position, const vismodule::OrthoSlice::AlignedAxis axis )
{
    SuperClass::setPlane( ::Normal[axis] * position, ::Normal[axis] );
}

} // end of namespace vismodule
