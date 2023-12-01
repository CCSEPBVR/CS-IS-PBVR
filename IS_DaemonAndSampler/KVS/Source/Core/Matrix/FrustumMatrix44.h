/*****************************************************************************/
/**
 *  @file   FrustumMatrix44.h
 *  @author Naohisa Sakamoto
 */
/*----------------------------------------------------------------------------
 *
 *  Copyright (c) Visualization Laboratory, Kyoto University.
 *  All rights reserved.
 *  See http://www.viz.media.kyoto-u.ac.jp/kvs/copyright/ for details.
 *
 *  $Id: FrustumMatrix44.h 602 2010-08-19 02:43:34Z naohisa.sakamoto $
 */
/*****************************************************************************/
#ifndef KVS__FRUSTUM_MATRIX44_H_INCLUDE
#define KVS__FRUSTUM_MATRIX44_H_INCLUDE

#include <cmath>
#include <kvs/Matrix44>
#include <kvs/Math>


namespace kvs
{

/*===========================================================================*/
/**
 *  @brief  Return view-frustum matrix (right-hand system).
 *  @param  left [in] Left position.
 *  @param  right [in] Right position.
 *  @param  bottom [in] Bottom position.
 *  @param  top [in] Top position.
 *  @param  near [in] Near position.
 *  @param  far [in] Far position.
 *  @return View-frustum matrix.
 */
/*===========================================================================*/
template <typename T>
inline kvs::Matrix44<T> FrustumMatrix44(
    const T left,
    const T right,
    const T bottom,
    const T top,
    const T near,
    const T far )
{
    KVS_ASSERT( !( kvs::Math::IsZero( right - left ) ) );
    KVS_ASSERT( !( kvs::Math::IsZero( top - bottom ) ) );
    KVS_ASSERT( !( kvs::Math::IsZero( far - near ) ) );

    const T elements[16] =
    {
        2 * near / ( right - left ),                           0,  ( right + left ) / ( right - left ),                                0,
                                  0, 2 * near / ( top - bottom ),  ( top + bottom ) / ( top - bottom ),                                0,
                                  0,                           0,     -( far + near ) / ( far - near ), -2 * far * near / ( far - near ),
                                  0,                           0,                                   -1,                                0
    };

    return( kvs::Matrix44<T>( elements ) );
};

} // end of namespace kvs

#endif // KVS__FRUSTUM_MATRIX44_H_INCLUDE
