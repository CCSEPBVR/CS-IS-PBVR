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
#ifndef VIS_MODULE__FRUSTUM_MATRIX44_H_INCLUDE
#define VIS_MODULE__FRUSTUM_MATRIX44_H_INCLUDE

#include <cmath>
#include <vismodule/Matrix44>
#include <vismodule/Math>


namespace vismodule
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
inline vismodule::Matrix44<T> FrustumMatrix44(
    const T left,
    const T right,
    const T bottom,
    const T top,
    const T near,
    const T far )
{
    VIS_MODULE_ASSERT( !( vismodule::Math::IsZero( right - left ) ) );
    VIS_MODULE_ASSERT( !( vismodule::Math::IsZero( top - bottom ) ) );
    VIS_MODULE_ASSERT( !( vismodule::Math::IsZero( far - near ) ) );

    const T elements[16] =
    {
        2 * near / ( right - left ),                           0,  ( right + left ) / ( right - left ),                                0,
                                  0, 2 * near / ( top - bottom ),  ( top + bottom ) / ( top - bottom ),                                0,
                                  0,                           0,     -( far + near ) / ( far - near ), -2 * far * near / ( far - near ),
                                  0,                           0,                                   -1,                                0
    };

    return( vismodule::Matrix44<T>( elements ) );
};

} // end of namespace vismodule

#endif // VIS_MODULE__FRUSTUM_MATRIX44_H_INCLUDE
