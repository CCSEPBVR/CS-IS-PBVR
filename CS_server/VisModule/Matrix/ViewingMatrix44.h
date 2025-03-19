/*****************************************************************************/
/**
 *  @file   ViewingMatrix.h
 *  @author Naohisa Sakamoto
 */
/*----------------------------------------------------------------------------
 *
 *  Copyright (c) Visualization Laboratory, Kyoto University.
 *  All rights reserved.
 *  See http://www.viz.media.kyoto-u.ac.jp/kvs/copyright/ for details.
 *
 *  $Id: ViewingMatrix44.h 602 2010-08-19 02:43:34Z naohisa.sakamoto $
 */
/*****************************************************************************/
#ifndef VIS_MODULE__VIEWING_MATRIX44_H_INCLUDE
#define VIS_MODULE__VIEWING_MATRIX44_H_INCLUDE

#include <cmath>
#include <vismodule/Matrix44>
#include <vismodule/Vector3>

namespace vismodule
{

/*===========================================================================*/
/**
 *  @brief  Return viewing matrix.
 *  @param  c [in] camera position in the world coordinate.
 *  @param  u [in] up-vector in the world coordinate
 *  @param  g [in] gaze point in the world coordinate
 *  @return Vewing matrix
 */
/*===========================================================================*/
template <typename T>
inline vismodule::Matrix44<T> ViewingMatrix44(
    const vismodule::Vector3<T>& c,
    const vismodule::Vector3<T>& u,
    const vismodule::Vector3<T>& g )
{
    vismodule::Vector3<T> d( g - c );
    vismodule::Vector3<T> r( d.cross( u ) );
    vismodule::Vector3<T> f( r.cross( d ) );

    d.normalize();
    r.normalize();
    f.normalize();

    const T elements[ 16 ] =
    {
         r.x(),  r.y(),  r.z(), -r.dot( c ),
         f.x(),  f.y(),  f.z(), -f.dot( c ),
        -d.x(), -d.y(), -d.z(),  d.dot( c ),
             0,      0,      0,             1
    };

    return( vismodule::Matrix44<T>( elements ) );
};

} // end of namespace vismodule

#endif // VIS_MODULE__VIEWING_MATRIX44_H_INCLUDE
