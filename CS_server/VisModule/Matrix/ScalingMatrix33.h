/*****************************************************************************/
/**
 *  @file   ScalingMatrix33.h
 *  @author Naohisa Sakamoto
 */
/*----------------------------------------------------------------------------
 *
 *  Copyright (c) Visualization Laboratory, Kyoto University.
 *  All rights reserved.
 *  See http://www.viz.media.kyoto-u.ac.jp/kvs/copyright/ for details.
 *
 *  $Id: ScalingMatrix33.h 602 2010-08-19 02:43:34Z naohisa.sakamoto $
 */
/*****************************************************************************/
#ifndef VIS_MODULE__SCALING_MATRIX33_H_INCLUDE
#define VIS_MODULE__SCALING_MATRIX33_H_INCLUDE

#include <cmath>
#include <vismodule/Matrix33>
#include <vismodule/Math>


namespace vismodule
{

/*===========================================================================*/
/**
 *  @brief  Returns a scaling matrix.
 *  @param  sx [in] Scaling factor of x-direction.
 *  @param  sy [in] Scaling factor of y-direction.
 *  @param  sz [in] Scaling factor of z-direction.
 *  @return Scaling matrix.
 */
/*===========================================================================*/
template <typename T>
inline vismodule::Matrix33<T> ScalingMatrix33( const T sx, const T sy, const T sz )
{
    const T zero = T(0);
    const T elements[ 9 ] =
    {
        sx,   zero, zero,
        zero,   sy, zero,
        zero, zero, sz
    };

    return( vismodule::Matrix33<T>( elements ) );
};

/*===========================================================================*/
/**
 *  @brief  Returns scaling matrix.
 *  @param  s [in] Scaling factor vector.
 *  @return Scaling matrix.
 */
/*===========================================================================*/
template <typename T>
inline vismodule::Matrix33<T> ScalingMatrix33( const vismodule::Vector3<T>& s )
{
    return( vismodule::ScalingMatrix33<T>( s.x(), s.y(), s.z() ) );
};

/*===========================================================================*/
/**
 *  @brief  Returns scaling matrix.
 *  @param  s [in] Scaling factor.
 *  @return Scaling matrix.
 */
/*===========================================================================*/
template <typename T>
inline vismodule::Matrix33<T> ScalingMatrix33( const T s )
{
    return( vismodule::ScalingMatrix33<T>( s, s, s ) );
};

} // end of namespace vismodule

#endif // VIS_MODULE__SCALING_MATRIX33_H_INCLUDE
