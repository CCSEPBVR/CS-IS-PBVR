/*****************************************************************************/
/**
 *  @file   LookAtMatrix44.h
 *  @author Naohisa Sakamoto
 */
/*----------------------------------------------------------------------------
 *
 *  Copyright (c) Visualization Laboratory, Kyoto University.
 *  All rights reserved.
 *  See http://www.viz.media.kyoto-u.ac.jp/kvs/copyright/ for details.
 *
 *  $Id: LookAtMatrix44.h 602 2010-08-19 02:43:34Z naohisa.sakamoto $
 */
/*****************************************************************************/
#ifndef VIS_MODULE__LOOK_AT_MATRIX44_H_INCLUDE
#define VIS_MODULE__LOOK_AT_MATRIX44_H_INCLUDE

#include <cmath>
#include <vismodule/Matrix44>
#include <vismodule/Math>


namespace vismodule
{

/*===========================================================================*/
/**
 *  @brief  Return look-at matrix.
 *  @param  eye [in] Camera(eye) position.
 *  @param  up [in] Camera's up-vector.
 *  @param  target [in] Target point.
 *  @return Look-at matrix.
 */
/*===========================================================================*/
template <typename T>
inline vismodule::Matrix44<T> LookAtMatrix44(
    const vismodule::Vector3<T>& eye,
    const vismodule::Vector3<T>& up,
    const vismodule::Vector3<T>& target )
{
    vismodule::Vector3<T> f( target - eye );
    vismodule::Vector3<T> s( f.cross( up.normalize() ) );
    vismodule::Vector3<T> u( s.cross( f ) );

    f.normalize();
    s.normalize();
    u.normalize();

    const T zero = T(0);
    const T one = T(1);
    const T elements[ 16 ] =
    {
         s.x(),  s.y(),  s.z(), zero,
         u.x(),  u.y(),  u.z(), zero,
        -f.x(), -f.y(), -f.z(), zero,
          zero,   zero,   zero, one
    };

    return( vismodule::Matrix44<T>( elements ) );
};

} // end of namespace vismodule

#endif // VIS_MODULE__TRANSFORMATION_H_INCLUDE
