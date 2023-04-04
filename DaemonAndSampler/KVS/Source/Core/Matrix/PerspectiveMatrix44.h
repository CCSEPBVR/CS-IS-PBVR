/*****************************************************************************/
/**
 *  @file   PerspectiveMatrix44.h
 *  @author Naohisa Sakamoto
 */
/*----------------------------------------------------------------------------
 *
 *  Copyright (c) Visualization Laboratory, Kyoto University.
 *  All rights reserved.
 *  See http://www.viz.media.kyoto-u.ac.jp/kvs/copyright/ for details.
 *
 *  $Id: PerspectiveMatrix44.h 602 2010-08-19 02:43:34Z naohisa.sakamoto $
 */
/*****************************************************************************/
#ifndef KVS__PERSPECTIVE_MATRIX44_H_INCLUDE
#define KVS__PERSPECTIVE_MATRIX44_H_INCLUDE

#include <cmath>
#include <kvs/Matrix44>
#include <kvs/Math>


namespace kvs
{

/*===========================================================================*/
/**
 *  @brief  Return perspective matrix (right-hand system).
 *  @param  fov [i] Field-of-view along a y-axis in degree.
 *  @param  aspect [in] Aspect ratio.
 *  @param  near [in] Near position.
 *  @param  far [in] Far position.
 */
/*===========================================================================*/
template <typename T>
inline kvs::Matrix44<T> PerspectiveMatrix44(
    const T fov,
    const T aspect,
    const T near,
    const T far )
{
    const T rad  = kvs::Math::Deg2Rad( fov / 2 );
    const T sinA = static_cast<T>( sin( rad ) );
    const T cosA = static_cast<T>( cos( rad ) );

    KVS_ASSERT( !( kvs::Math::IsZero( sinA ) ) );
    KVS_ASSERT( !( kvs::Math::IsZero( aspect ) ) );
    KVS_ASSERT( !( kvs::Math::IsZero( far -near ) ) );

    const T cotA = cosA / sinA;
    const T elements[16] =
    {
        cotA / aspect,    0,                                0,                                    0,
                    0, cotA,                                0,                                    0,
                    0,    0, -( far + near ) / ( far - near ), -( far * near * 2 ) / ( far - near ),
                    0,    0,                               -1,                                    1
    };

    return( kvs::Matrix44<T>( elements ) );
};

} // end of namespace kvs

#endif // KVS__PERSPECTIVE_MATRIX44_H_INCLUDE
