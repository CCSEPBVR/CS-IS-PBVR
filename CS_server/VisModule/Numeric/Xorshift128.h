/****************************************************************************/
/**
 *  @file Xorshift128.h
 */
/*----------------------------------------------------------------------------
 *
 *  Copyright (c) Visualization Laboratory, Kyoto University.
 *  All rights reserved.
 *  See http://www.viz.media.kyoto-u.ac.jp/kvs/copyright/ for details.
 *
 *  $Id: Xorshift128.h 634 2010-10-13 07:04:05Z naohisa.sakamoto $
 */
/****************************************************************************/
#ifndef VIS_MODULE__XORSHIFT_128_H_INCLUDE
#define VIS_MODULE__XORSHIFT_128_H_INCLUDE

#include <vismodule/ClassName>
#include <vismodule/Type>


namespace vismodule
{

/*==========================================================================*/
/**
 *  Xorshift128 class.
 */
/*==========================================================================*/
class Xorshift128
{
    visModuleClassName_without_virtual( vismodule::Xorshift128 );

private:

    vismodule::UInt32 m_x;
    vismodule::UInt32 m_y;
    vismodule::UInt32 m_z;
    vismodule::UInt32 m_w;

public:

    Xorshift128( void );

public:

    void setSeed( vismodule::UInt32 );

public:

    const float rand( void );

public:

    const float operator ()( void );
};

/*==========================================================================*/
/**
 *  .
 *
 *  @return TODO
 */
/*==========================================================================*/
inline const float Xorshift128::rand( void )
{
    const float t24 = 1.0 / 16777216.0; /* 0.5**24 */

    vismodule::UInt32 t = ( m_x ^ ( m_x << 11 ) );

    m_x = m_y;
    m_y = m_z;
    m_z = m_w;
    m_w = ( m_w ^ ( m_w >> 19 ) ) ^ ( t ^ ( t >> 8 ) );

    return( t24 * static_cast<float>( m_w >> 8 ) );
}

/*==========================================================================*/
/**
 *  .
 *
 *  @return TODO
 */
/*==========================================================================*/
inline const float Xorshift128::operator ()( void )
{
    return( this->rand() );
}

} // end of namespace vismodule

#endif // VIS_MODULE__XORSHIFT_128_H_INCLUDE
